#include <GRT.h>
#include <iostream>
#include <string>
#include <locale> // for isspace
#include <functional>
#include <math.h>
#include "cmdline.h"

using namespace GRT;

int train(cmdline::parser&);
int predict(cmdline::parser&);
int create(cmdline::parser&);

InfoLog info;

int main(int argc, char *argv[])
{
  cmdline::parser c;
  c.add<string>("model",   'm', "GRT model file", true);
  c.add<int>   ("verbose", 'v', "verbosity level: 0-4", 0);
  c.add        ("help",    'h', "print this message");

  // second argument must be command
  const char* cmd = argc<2 ? "none" : argv[1];
  int (*action)(cmdline::parser &c) = NULL;
  const char *cmdhelp = "";

  if (strcmp(cmd, "train")==0) {
    c.add<string>("data",     'd', "data file to load, opens up stdin if not given", false, "-");
    c.add<string>("datatype", 't', "force classification, regression or timeseries input", false);
    c.add<int>("k-folds",     'k', "number of cross-validation folds, should be in the range of [1,numTrainingSamples-1], default is no cross-validation", false, 1);
    c.add("stratified",       's', "wether samples should be stratified during cross-validation");
    c.add("report",           'r', "report cross-validation results if done");

    cmdhelp = "trains the model on the supplied data file.\n"
              "If a Regression, Classification or Timeseries Classfication is to be used is deduced from the file format"
              ;
    action  = train;
  } else if (strcmp(cmd, "predict")==0) {
    c.add<string>("data",     'd', "data file to load, opens up stdin if not given", false, "-");
    c.add<string>("datatype", 't', "force classification, regression or timeseries input", false);

    cmdhelp = "predict labels for incomcing data, if labels are given in the datastream then it can also be used for validation";
    action = predict;
  } else if (strcmp(cmd, "create")==0) {
    c.add<string>("preprocessor",  'p', "list of preprocessing modules", false);
    c.add<string>("feature",       'f', "list of feature extraction modules", false);
    c.add<string>("classifier",    'c', "classifier module", false);
    c.add<string>("postprocessor", 'q', "list of post-processing modules", false);

    cmdhelp = "creates a template pipeline in the <recognizer> file.\n"
              "Preprocessing, Feature Extraction and Postprocessing can be comma-separated lists.\n"
              "Only one recognizer is allowed for each pipeline\n\n"
              "Supplying \"list\" as one of the module will list all available";
    action = create;
  } else {
    std:cerr << c.usage() << "\n"
             << "\n"
             << "The available commands:\n"
             << "  train   - train a recognition pipeline\n"
             << "  predict - use a pipeline for prediction\n"
             << "  create  - creates a template pipeline\n"
             << "\n"
             << "help for each command can be retrieved with -h\n"
             ;
     return -1;
  };

  if (!c.parse(argc, argv)) {
    std::cerr << c.usage() << "\n" << c.error() << "\n" ;
    return -1;
  } else if (c.exist("help")) {
    std::cerr << c.usage() << "\n" << cmdhelp << "\n" ;
    return 0;
  }

  DebugLog::enableLogging(false);
  TrainingLog::enableLogging(false);
  TestingLog::enableLogging(false);
  InfoLog::enableLogging(false);
  WarningLog::enableLogging(false);
  ErrorLog::enableLogging(false);
  switch(c.get<int>("verbose")) {
      case 4: DebugLog::enableLogging(true);
      case 3: TestingLog::enableLogging(true); TrainingLog::enableLogging(true);
      case 2: InfoLog::enableLogging(true);
      case 1: WarningLog::enableLogging(true);
      case 0: ErrorLog::enableLogging(true);
  }

  return action(c);
}

int create(cmdline::parser &args) {
  GestureRecognitionPipeline pipeline;
  std::string arg_classifier    = args.get<string>("classifier"),
              arg_outputfile    = args.get<string>("model"),
              arg_preprocessor  = args.get<string>("preprocessor"),
              arg_postprocessor = args.get<string>("postprocessor"),
              arg_feature       = args.get<string>("feature");

  if (args.exist("classifier")) {
    Classifier *classifier = Classifier::createInstanceFromString(arg_classifier);
    if (classifier == NULL) {
      std:cerr  << "classifier not found" << endl;
      std::cout << "available classifiers:" << endl;
      std::vector<string> names = Classifier::getRegisteredClassifiers();
      for(std::vector<string>::iterator it = names.begin(); it != names.end(); ++it) {
        std::cout << *it << endl;
      }

      return 0;
    }

    if (!pipeline.setClassifier( *classifier ))
      return -1;
  }

  // add preprocs
  if (args.exist("preprocessor")) {
    for (int end = (arg_preprocessor.find(",") > 0 ? arg_preprocessor.find(",") : arg_preprocessor.length()),
             pos = 0;
             pos <= arg_preprocessor.length();
             pos = end==-1 ? -1 : end+1,
             end = arg_preprocessor.find(",",end+1) ) {
      PreProcessing *preprocessor = PreProcessing::createInstanceFromString( arg_preprocessor.substr(pos,end) );

      if (preprocessor == NULL) {
        std::cerr << "no preprocessing module with the name: " << arg_preprocessor.substr(pos,end) << endl;
        std::cout << "available preprocessors" << endl;
        std::vector<string> names = PreProcessing::getRegisteredPreprocessors();
        for(std::vector<string>::iterator it = names.begin(); it != names.end(); ++it) {
          std::cout << *it << endl;
        }
        return -1;
      }

      if (!pipeline.addPreProcessingModule( *preprocessor ))
        return -1;
    }
  }

  // add feature extractors
  if (args.exist("feature")) {
    for (int end = (arg_feature.find(",") > 0 ? arg_feature.find(",") : arg_feature.length()),
             pos = 0;
             pos <= arg_feature.length();
             pos = end==-1 ? -1 : end+1,
             end = arg_feature.find(",",end+1) ) {
      FeatureExtraction *feature = FeatureExtraction::createInstanceFromString( arg_feature.substr(pos,end) );

      if (feature == NULL) {
        std::cerr << "no feature extracting module with the name: " << arg_feature.substr(pos,end) << endl;
        std::cout << "available feature extractors" << endl;
        std::vector<string> names = FeatureExtraction::getRegisteredFeatureExtractors();
        for(std::vector<string>::iterator it = names.begin(); it != names.end(); ++it) {
          std::cout << *it << endl;
        }
        return -1;
      }

      if (!pipeline.addFeatureExtractionModule( *feature ))
        return -1;
    }
  }

  // add post-procs
  if (args.exist("postprocessor")) {
    for (int end = (arg_postprocessor.find(",") > 0 ? arg_postprocessor.find(",") : arg_postprocessor.length()),
             pos = 0;
             pos <= arg_postprocessor.length();
             pos = end==-1 ? -1 : end+1,
             end = arg_postprocessor.find(",",end+1) ) {
      PostProcessing *postprocessor = PostProcessing::createInstanceFromString( arg_postprocessor.substr(pos,end) );

      if (postprocessor == NULL) {
        std::cerr << "no postprocessing module with the name: " << arg_postprocessor.substr(pos,end) << endl;
        std::cout << "available postprocessors" << endl;
        std::vector<string> names = PostProcessing::getRegisteredPostProcessors();
        for(std::vector<string>::iterator it = names.begin(); it != names.end(); ++it) {
          std::cout << *it << endl;
        }
        return -1;
      }

      if (!pipeline.addPostProcessingModule( *postprocessor ))
        return -1;
    }
  }

  return pipeline.savePipelineToFile( arg_outputfile ) ? 0 : -1;
}

bool iscomment(std::string line) {
  for (int i=0; i<line.length(); i++) {
    char c = line[i];
    if (std::isspace(c))
      continue;
    return c=='#';
  }
}

bool isempty(std::string line) {
  for (int i=0; i<line.length(); i++) {
    char c=line[i];
    if (!std::isspace(c))
      return false;
  }
  return true;
}

bool isLabelANumber(std::string label) {
  try { std::stod(label); return true; }
  catch(exception &e) { return false; }
}

enum loadData_ret_TYPE { FAILED, UNLABELLED, REGRESSION, CLASSIFICATION, TIMESERIES };
class loadData_ret {
  public:
  enum loadData_ret_TYPE type;
  UnlabelledData u_data;
  RegressionData r_data;
  ClassificationData c_data;
  TimeSeriesClassificationData t_data;
  std::vector<string> labels = std::vector<string>(1, "NULL");
  int classkey(std::string label) {
    int i = std::find(labels.begin(), labels.end(), label) == labels.end() ? -1 :
            std::find(labels.begin(), labels.end(), label) - labels.begin();
    if (i==-1) {
      labels.push_back(label);
      return labels.size()-1;
    }
    return i;
  }
};
loadData_ret
loadData(std::istream &file, std::string datatype="none", bool canBeUnlabelled=true)
{
  loadData_ret container;
  container.type = FAILED;

  if ((datatype!="none" && datatype!="") && !(datatype=="classification" || datatype=="regression" || datatype=="timeseries")) {
    std:cerr << "datatype must be one of: classification, regression or timeseries" << endl;
    return container;
  }

  // load all data into a generic type
  // to either Regression or Classification data after we read the whole file
  typedef std::pair<std::string, std::vector<double>> sample_t;
  typedef std::vector<sample_t> block_t;
  typedef std::vector<block_t> blocks_t;

  blocks_t blocks(1, block_t(0));
  std::string line;
  int lineNum = 0;

  while (std::getline(file, line)) {
    lineNum++;

    if (iscomment(line))
      continue;

    if (isempty(line)) { // got a block
      if (blocks.back().size() == 0)
        continue; // more than one empty line

      block_t block;
      blocks.push_back(block);
      continue;
    }

    std::string token;
    std::stringstream ss(line);
    sample_t sample("", std::vector<double>());

    ss >> token; sample.first = token;

    while (ss >> token) {
      try { sample.second.push_back( std::stod(token) ); }
      catch(exception &e) {
        std::cerr << "unable to parse input (not a number) at line " << lineNum << endl;
        return container;
      }
    }

    blocks.back().push_back(sample);
  }

  if (blocks.back().size() == 0) // was a trailing empty line
    blocks.erase(blocks.end() - 1 );

  // now check wether we have a classifier, timeseries or regression set
  // regression if labels are numbers and data dimension is 1, blocks are
  // samples
  //
  // timeseries if block length differ, but labels in each block are the same
  //
  // classification if labels are not numbers and data dimension is the same for
  // each sample, blocks are ignored
  bool all_lables_are_numbers = true,
       labels_in_block_are_equal = true,
       all_data_dimensions_are_equal = true,
       all_block_dimensions_are_equal = true,
       data_dimension_is_1 = true;
  int block_dimension = -1;

  for (blocks_t::iterator block=blocks.begin(); block!=blocks.end(); ++block) {
    std::string label="";
    int dimension=-1;

    all_block_dimensions_are_equal = all_block_dimensions_are_equal &&
                                     (block_dimension==-1 ||
                                      block_dimension==block->size());
    block_dimension=block->size();

    for (block_t::iterator sample=block->begin(); sample!=block->end(); ++sample) {
      all_lables_are_numbers = all_lables_are_numbers &&
                               isLabelANumber(sample->first);
      labels_in_block_are_equal = labels_in_block_are_equal &&
                                 (label=="" || label==sample->first);
      label = sample->first;

      all_data_dimensions_are_equal = all_data_dimensions_are_equal &&
                                     (dimension==-1 || dimension==sample->second.size());
      data_dimension_is_1 = data_dimension_is_1 && dimension==1;
      dimension = sample->second.size();
    }
  }

  // check the datatype argument
  if ( datatype!="classification" && datatype!="regression" && datatype!="timeseries" && datatype!="unlabelled" )
    if ( all_lables_are_numbers )
      datatype = "regression";
    else if (all_block_dimensions_are_equal)
      datatype = "classification";
    else if (labels_in_block_are_equal)
      datatype = "timeseries";
    else {
      std::cerr << "could not guess datatype from input, please specify with -t" << endl;
      return container;
    }

  // load into dataset
  if ( datatype=="regression" ) {
    std::cerr << "regression not implemented yet" << endl;
  } else if ( datatype=="classification") {
    ClassificationData dataset( blocks.back().back().second.size() );
    dataset.setAllowNullGestureClass( true );

    for (blocks_t::iterator block=blocks.begin(); block!=blocks.end(); ++block) {
      for (block_t::iterator sample=block->begin(); sample!=block->end(); ++sample) {
        std::string label = sample->first;
        int label_index = container.classkey(label);

        dataset.addSample(label_index, sample->second);
        dataset.setClassNameForCorrespondingClassLabel(label, label_index);
      }
    }

    info << dataset.getStatsAsString();

    container.type = CLASSIFICATION;
    container.c_data = dataset;
    return container;
  } else if ( datatype=="timeseries" ) {
    TimeSeriesClassificationData dataset( blocks.back().back().second.size() );
    dataset.setAllowNullGestureClass( true );
    dataset.setClassNameForCorrespondingClassLabel("NULL", 0);

    for (blocks_t::iterator block=blocks.begin(); block!=blocks.end(); ++block) {
      // cols == sample.size()
      std::vector< std::vector <double> > vals;
      MatrixDouble md(block->size(), block->back().second.size());
      std::string label = block->back().first;
      int label_index = container.classkey(label);

      for (block_t::iterator sample=block->begin(); sample!=block->end(); ++sample)
        vals.push_back(sample->second);

      md = vals;
      dataset.addSample(label_index,md);
      dataset.setClassNameForCorrespondingClassLabel(label, label_index);
    }

    info << dataset.getStatsAsString();

    container.type = TIMESERIES;
    container.t_data = dataset;
    return container;
  }

  return container;
}

std::string report(std::vector< TestResult > ts, std::vector< string > labels)
{
  using namespace std::placeholders;
  std::stringstream ss;
  #define meanof(member, collection) (std::accumulate(collection.begin(),collection.end(), 0, [=](double result, TestResult t) -> double { return  result + t.member; }) / collection.size() )
  #define stddev(member, collection) (sqrt(std::accumulate(collection.begin(),collection.end(), 0, std::bind([=](double result, TestResult t, double mean) -> double { return result + (mean - t.member); },_1,_2,meanof(member,collection))) / collection.size()) )
  #define stat(member) meanof(member, ts) << "±" << stddev(member,ts)
  #define stats(member) ""#member": " << meanof(member, ts) << "±" << stddev(member,ts)

  ss << stats(numTrainingSamples) << endl;
  ss << stats(numTestSamples) << endl;
  ss << stats(accuracy) << endl;
  ss << stats(rmsError) << endl;
  ss << stats(totalSquaredError) << endl;
  ss << stats(testTime) << endl;
  ss << stats(trainingTime) << endl;
  ss << stats(rejectionRecall) << endl;
  ss << stats(rejectionPrecision) << endl;

  ss << endl;
  ss << "\tprecision\trecall\tF1" << endl;

  bool HAS_NO_NULL_CLASS = ts[0].precision.size()!=labels.size();

  for (int i=0; i<ts[0].precision.size(); i++) {
    double mean_precision = meanof(precision[i], ts),
           mean_recall = meanof(recall[i], ts);

    ss << labels[i+HAS_NO_NULL_CLASS]
       << "\t" << stat(precision[i])
       << "\t" << stat(recall[i])
       << "\t" << 2*((mean_precision*mean_recall)/(mean_recall+mean_precision))
       << endl;
  }

  ss << endl;

  MatrixDouble md = ts[0].confusionMatrix;
  for (int i=1; i<ts.size(); i++)
    md.add(ts[i].confusionMatrix);

  for (int i=HAS_NO_NULL_CLASS; i<labels.size(); i++)
    ss << "\t" << labels[i];
  ss << endl;

  for (int i=0; i<md.getNumRows(); i++) {
    ss << labels[i+HAS_NO_NULL_CLASS];
    for (int j=0; j<md.getNumRows(); j++)
      ss << "\t" << md[i][j];
    ss << endl;
  }

  return ss.str();

}

int train(cmdline::parser &args) {
  string model = args.get<string>("model"),
         datafilename = args.get<string>("data"),
         datatype = args.get<string>("datatype");
  bool stratitified = args.exist("stratified");
  int kfold = args.get<int>("k-folds");

  if (args.exist("stratified") && !args.exist("k-folds")) {
    std::cerr << "need both stratified and k-folds argument" << endl;
    return -1;
  }

  if (args.exist("report") && !args.exist("k-folds")) {
    std::cerr << "need both stratified and k-folds argument" << endl;
    return -1;
  }

  GestureRecognitionPipeline pipeline;
  if (!pipeline.load(model)) {
    std::cerr << "unable to load model from file: " << model << endl;
    return -1;
  }

  fstream file;
  file.open(datafilename.c_str(), iostream::in);
  if (!file.is_open()) {
      std::cerr << "could not open data file: " << datafilename << endl;
      return -1;
   }

  bool ok = false;
  loadData_ret container = loadData(file, datatype);

  switch( container.type ) {
    case TIMESERIES:
      ok = !args.exist("k-folds") ? pipeline.train(container.t_data) :
           pipeline.train(container.t_data, args.get<int>("k-folds"), args.exist("stratified"));
      ok &= pipeline.save(model);
      break;
    case CLASSIFICATION:
      ok = !args.exist("k-folds") ? pipeline.train(container.c_data) :
           pipeline.train(container.c_data, args.get<int>("k-folds"), args.exist("stratified"));
      ok &= pipeline.save(model);
      break;
    default:
      std::cerr << "container type not supported" << endl;
      break;
  }

  if (!ok) {
    std::cerr << "training failed" << endl;
    return -1;
  }

  if (args.exist("report")) {
    if (pipeline.getCrossValidationResults().size() != 0)
      std::cout << report( pipeline.getCrossValidationResults(), container.labels );
    else
    {
      std::vector < TestResult > v = std::vector<TestResult>(1, pipeline.getTestResults());
      std::cout << report( v, container.labels );
    }
  }

  return 0;
}

int predict(cmdline::parser &args) {
  string model = args.get<string>("model"),
         datafilename = args.get<string>("data"),
         datatype = args.get<string>("datatype");

  GestureRecognitionPipeline pipeline;
  if (!pipeline.load(model)) {
    std::cerr << "unable to load model from file: " << model << endl;
    return -1;
  }

  fstream file;
  file.open(datafilename.c_str(), iostream::in);
  if (!file.is_open()) {
      std::cerr << "could not open data file: " << datafilename << endl;
      return -1;
   }

  loadData_ret container = loadData(file, datatype);
  switch( container.type ) {
    case TIMESERIES:
      for (int i=0; i<container.t_data.getClassificationData().size(); i++)
      {
        int actual = container.t_data.getClassificationData()[i].getClassLabel(),
        prediction = pipeline.predict( container.t_data.getClassificationData()[i].getData() );

        std::cout << container.t_data.getClassNameForCorrespondingClassLabel(prediction)
                  << " "
                  << container.t_data.getClassNameForCorrespondingClassLabel(actual)
                  << endl;
      }
      break;
    case CLASSIFICATION:
      for (int i=0; i<container.c_data.getClassificationData().size(); i++)
      {
        int actual = container.c_data.getClassificationData()[i].getClassLabel(),
        prediction = pipeline.predict( container.c_data.getClassificationData()[i].getSample() );

        std::cout << container.c_data.getClassNameForCorrespondingClassLabel(prediction)
                  << " "
                  << container.c_data.getClassNameForCorrespondingClassLabel(actual)
                  << endl;
      }
      break;
    case REGRESSION:
      std::cerr << "regression not supported yet" << endl;
      break;
    case UNLABELLED:
      std::cerr << "unlabelled not supported yet" << endl;
      break;
    default:
      std::cerr << "unable to load data" << endl;
  }

  return -1;
}
