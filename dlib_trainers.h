/*
 * Author: Sebastian Boettcher
 *
 * Conveniance wrappers for some dlib trainers.
 *
 */

#ifndef _DLIB_TRAINERS_H_
#define _DLIB_TRAINERS_H_

#include <dlib/svm.h>
#include <dlib/svm_threaded.h>

#include <unordered_map>

#include "enum.h"

using namespace std;
using namespace dlib;


BETTER_ENUM(TrainerType, int,
  TEMPLATE,
  BINARY,
  MULTICLASS,
  REGRESSION
)

BETTER_ENUM(TrainerName, int,
  TEMPLATE,
  ONE_VS_ONE,
  ONE_VS_ALL,
  SVM_LINEAR
)

multimap<TrainerType, TrainerName> type_map = {
  {TrainerType::MULTICLASS, TrainerName::ONE_VS_ONE},
  {TrainerType::MULTICLASS, TrainerName::ONE_VS_ALL},
  {TrainerType::MULTICLASS, TrainerName::SVM_LINEAR}
};

void printTrainers() {
  for (TrainerType t_type : TrainerType::_values()) {
    if (type_map.count(t_type))
      cout << t_type << endl;
    for (auto const & kv : type_map)
      if (kv.first == t_type)
        cout << "\t" << kv.second << endl;
  }
}



// typedef for one sample, init as 0,1 ; can be cast to arbitrary num_rows
typedef matrix<double, 0, 1> sample_type;
typedef string label_type;

typedef std::vector<sample_type> v_sample_type;
typedef std::vector<label_type> v_label_type;


// one vs one trainer typedefs
typedef one_vs_one_trainer<any_trainer<sample_type>, label_type> ovo_trainer_type;
typedef radial_basis_kernel<sample_type> rbf_kernel;
typedef one_vs_one_decision_function<ovo_trainer_type, decision_function<rbf_kernel>> ovo_trained_function_type;


//_______________________________________________________________________________________________________
class trainer_template {
 public:
  trainer_template(){}

  TrainerType getTrainerType() { return m_trainer_type; }
  TrainerName getTrainerName() { return m_trainer_name; }

  // These functions should be implemented!
  // (but can't be declared here because types change)

  //virtual any_trainer<sample_type, string> getTrainer() = 0;
  //virtual any_decision_function<sample_type, label_type> train(const v_sample_type& all_samples, const v_label_type& all_labels) const = 0;


  virtual matrix<double> crossValidation(std::vector<sample_type> samples, std::vector<label_type> labels, size_t k = 5) = 0;

 protected:
  void setTrainerType(TrainerType type) { m_trainer_type = type; }
  void setTrainerName(TrainerName name) { m_trainer_name = name; }

 private:
  TrainerType m_trainer_type = TrainerType::TEMPLATE;
  TrainerName m_trainer_name = TrainerName::TEMPLATE;

};



//_______________________________________________________________________________________________________
class ovo_trainer : public trainer_template {
 public:
  ovo_trainer(int num_threads = 4, double kernel_gamma = 0.1) {
    setTrainerType(TrainerType::MULTICLASS);
    setTrainerName(TrainerName::ONE_VS_ONE);

    ovo_trainer_type tmp;

    krr_trainer<rbf_kernel> krr_rbf_trainer;
    krr_rbf_trainer.set_kernel(rbf_kernel(kernel_gamma));

    tmp.set_trainer(krr_rbf_trainer);
    tmp.set_num_threads(num_threads);

    m_trainer = tmp;
  }

  ovo_trainer_type getTrainer() { return m_trainer; }

  ovo_trained_function_type train (const v_sample_type& all_samples, const v_label_type& all_labels) const {
    return m_trainer.train(all_samples, all_labels);
  }

  virtual matrix<double> crossValidation(v_sample_type samples, v_label_type labels, size_t k = 5) {
    return cross_validate_multiclass_trainer(m_trainer, samples, labels, k);
  }

 private:

  ovo_trainer_type m_trainer;

};


#endif // _DLIB_TRAINERS_H_
