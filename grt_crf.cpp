#include "grt_crf.h"
#include <stdlib.h>

using namespace GRT;

namespace GRT {

RegisterClassifierModule< CRF > CRF::registerModule("CRF");

CRF::CRF(string algorithm, int minfreq, bool generate_all_states, bool generate_all_transitions) :
  algorithm(algorithm), minfreq(minfreq), generate_all_states(generate_all_states),
  generate_all_transitions(generate_all_transitions) 
{
  classifierMode = TIMESERIES_CLASSIFIER_MODE;
  classType = "CRF";
}

/** Copy Constructor */
CRF::CRF(CRF &rhs) {
  classifierMode = rhs.classifierMode;
  classType = rhs.classType;
  algorithm = rhs.algorithm;
  minfreq = rhs.minfreq;
  generate_all_states = rhs.generate_all_states;
  generate_all_transitions = rhs.generate_all_transitions;
}


/** Destructor */
CRF::~CRF() {
}

/** return non-zero to stop training */
int training_msg_cb(void *instance, const char *fmt, va_list args) {
  static TrainingLog t;
  char str[1024];
  vsnprintf(str,sizeof(str),fmt,args);
  t << str;
  return 0;
}

/** only timeseries is supported in this classifier */
bool CRF::train_(TimeSeriesClassificationData &ts) {
  crfsuite_dictionary_t *attrs, *labels;
  crfsuite_trainer_t *t;
  crfsuite_data_t data;

  crfsuite_data_init(&data);
  if (!(crfsuite_create_instance("dictionary", (void**) &data.attrs) &&
        crfsuite_create_instance("dictionary", (void**) &data.labels) )) {
    errorLog << "unable to create label or attribute dictionary" << endl;
    return false;
  }

  attrs  = data.attrs;
  labels = data.labels;

  if (ts.getNumClasses() <= 1) {
    errorLog << "CRF needs more than one classlabel in training set" << endl;
    return false;
  }

  if (ts.getNumSamples() == 0) {
    errorLog << " CRF needs at least one training sample" << endl;
    return false;
  }


  // TODO t should be freed at some point
  char trainer_id[256];
  snprintf(trainer_id, sizeof(trainer_id), "train/%s/%s", "crf1d","lbfgs");
  if (!crfsuite_create_instance(trainer_id, (void**) &t)) {
   errorLog << "unable to create trainer " << trainer_id << endl;
    return false;
  }

  if (ts.getNumSamples() > 0) {
    TimeSeriesClassificationSample sample = ts[0];

    char str[256];
    string label;
    crfsuite_item_t item;
    crfsuite_attribute_t attr;
    crfsuite_instance_t crfsuite_sample;

    crfsuite_item_init(&item);
    crfsuite_instance_init(&crfsuite_sample);

    /* load all label into the labels store of CRFSuite */
    for (int i=0; i<ts.getNumClasses(); i++) {
      string label = ts.getClassNameForCorrespondingClassLabel(i + ts.getMinimumClassLabel());
      labels->get(labels,label.c_str());
    }

    for (size_t i=0; i<ts.getNumSamples(); i++)
    {
      sample = ts[i];
      label = ts.getClassNameForCorrespondingClassLabel(sample.getClassLabel());

      if (sample.getData().getNumCols() != 1) {
        errorLog << "can only work with 1-dimensional data (maybe use one of the Quantizer modules)" << endl;
        return false;
      }

      for (size_t j=0; j<sample.getData().getNumRows(); j++) {
        snprintf(str,sizeof(str),"%f",sample.getData()[j][0]);
        attr.value = sample.getData()[j][0];
        attr.aid   = attrs->get(attrs, str);
        crfsuite_item_append_attribute(&item, &attr);
        crfsuite_instance_append(&crfsuite_sample,&item,sample.getClassLabel() - ts.getMinimumClassLabel());
        crfsuite_item_finish(&item);
      }

      crfsuite_data_append(&data, &crfsuite_sample);
      crfsuite_instance_finish(&crfsuite_sample);
    }
  }

  t->set_message_callback(t, this, training_msg_cb);

  strcpy(tmpFileName, "/tmp/grtcrfXXXXXX");
  if (!mkstemp(tmpFileName)) {
    errorLog << "unable to create tmp file " << tmpFileName << endl;
    return false;
  }

  if (t->train(t,&data,tmpFileName,-1)) {
    errorLog << "training CRF failed" << endl;
    return false;
  }

  trained = true;
  return true;
}


bool CRF::predict(VectorDouble inputVector) {
  errorLog << "CRF does not support classification" << endl;
  return false;
}

/** override the prediction */
bool CRF::predict_(MatrixDouble &sample) {
  crfsuite_tagger_t *tagger;
  crfsuite_instance_t inst;
  crfsuite_item_t item;
  crfsuite_attribute_t attr;
  crfsuite_dictionary_t *labels, *attrs;

  crfsuite_instance_init(&inst);

  if (!model) {
    errorLog << "CRF instance is not trained" << endl;
    return false;
  }

  // TODO this can be optimized out
  if (model->get_tagger(model, &tagger)) {
    errorLog << "unable to get tagger from CRF model" << endl;
    return false;
  }

  if (model->get_labels(model, &labels)) {
    errorLog << "unable to get labels from CRF model" << endl;
    return false;
  }

  if (model->get_attrs(model, &attrs)) {
    errorLog << "unable to get attributes from CRF model" << endl;
    return false;
  }

  if (sample.getNumCols() != 1) {
    errorLog << "only 1-dimensional input is supported" << endl;
    return false;
  }

  char str[256];
  for (int j=0; j<sample.getNumRows(); j++) {
    crfsuite_item_init(&item);
    snprintf(str,sizeof(str),"%f",sample[j][0]);
    attr.value = sample[j][0];
    attr.aid   = attrs->to_id(attrs, str);
    crfsuite_item_append_attribute(&item, &attr);
    crfsuite_instance_append(&inst,&item,0);
    crfsuite_item_finish(&item);
  }

  if (inst.num_items == 0) {
    errorLog << "empty instance" << endl;
    return false;
  }

  if (tagger->set(tagger, &inst)) {
    errorLog << "unable to set instance on tagger" << endl;
    return false;
  }

  int *output = (int*) calloc(sizeof(int), inst.num_items);
  floatval_t score;

  if (tagger->viterbi(tagger, output, &score)) {
    errorLog << "unable to predict" << endl;
    return false;
  }

  // XXX maybe bag-of-words here?, also the +1 is magical for filtering the NULL
  // class (all class labels in GRT start at 1 not 0!)
  predictedClassLabel = output[0] + 1; 

  free(output);
  return true;
}

bool CRF::reset() {
  model = NULL;
}

bool CRF::clear() { reset(); }
bool CRF::print() const {}

bool CRF::saveModelToFile(fstream &f) const {
  std::ifstream in(tmpFileName, ios::binary);

  // TODO remove tmpfile

  if (!f.is_open()) {
    errorLog << "saveModelToFile() - output file not open" << endl;
    return false;
  }

  if (!in.good()) {
    errorLog << "saveModelToFile() - model has not been trained yet" << endl;
    return false;
  }

  f << in.rdbuf();
  f.close();
  in.close();

  return true;
}

#define MAGIC "lCRF"
bool CRF::loadModelFromFile(string f) {
  char header[4];
  crfsuite_dictionary_t *labels;
  ifstream is(f, ios::in|ios::binary);
  int red = 0;

  while (red < 4) {
    is.read(header+red, sizeof(header)-red);
    red += is.tellg();
  }

  if (!is || memcmp(MAGIC,header,4)!=0)
    return false;

  trained = !crfsuite_create_instance_from_file(f.c_str(), (void**) &model);

  if (!model->get_labels(model, &labels)) {
    char *label;

    for (int i=0; i<labels->num(labels); i++) {
      classLabels.push_back(i);
      labels->to_string(labels,i,(const char**) &label);
      setClassNameForLabel(i, label);
    }
  }

  return trained;
}
}
