#ifndef _GRT_CRF_H_
#define _GRT_CRF_H_

#include <GRT.h>
#include <crfsuite.h>
#include <cstdio>

namespace GRT {
class CRF : public Classifier {
public:
  /** Default Constructor */
  CRF(string algorithm = "lbfgs", int minfreq=0., bool generate_all_states=false, bool generate_all_transitions=false);

  /** Copy Constructor */
  CRF(CRF &);

  /** Destructor */
  virtual ~CRF();

  /** only timeseries is supported in this classifier */
  virtual bool train_(TimeSeriesClassificationData &);

  /** override the prediction */
  virtual bool predict_(MatrixDouble &);
  virtual bool predict(VectorDouble inputVector);

  virtual bool reset();
  virtual bool clear();
  virtual bool print() const;
  virtual bool saveModelToFile(ostream& ) const override;
  virtual bool loadModelFromFile(string);

  using MLBase::saveModelToFile;
  using MLBase::loadModelFromFile;
  using MLBase::train_;
  using MLBase::predict_;

protected:
  crfsuite_model_t *model = NULL;
  char tmpFileName[L_tmpnam];

  static RegisterClassifierModule< CRF > registerModule;

  string algorithm;
  int minfreq;
  bool generate_all_states, generate_all_transitions;

  crfsuite_dictionary_t *labels = NULL, *attrs = NULL;
  crfsuite_tagger_t *tagger = NULL;
};
}

#endif
