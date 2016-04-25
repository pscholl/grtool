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


/*
 *  ENUMS
 */

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
  SVM_MULTICLASS_LINEAR
)

multimap<TrainerType, TrainerName> type_name_map = {
  {TrainerType::MULTICLASS, TrainerName::ONE_VS_ONE},
  {TrainerType::MULTICLASS, TrainerName::ONE_VS_ALL},
  {TrainerType::MULTICLASS, TrainerName::SVM_MULTICLASS_LINEAR}
};

void printTrainers() {
  for (TrainerType t_type : TrainerType::_values()) {
    if (type_name_map.count(t_type))
      cout << t_type << endl;
    for (auto const & kv : type_name_map)
      if (kv.first == t_type)
        cout << "\t" << kv.second << endl;
  }
}

bool classifierExists(string name) {
  for (const char * tname : TrainerName::_names())
    if (name.compare(tname) == 0)
      return true;
  return false;
}

bool operator==(const std::string& name_str, const TrainerName& name) {
  return TrainerName::_from_string(name_str.c_str()) == name;
}


/*
 *  TYPEDEFS
 */

// typedef for one sample, init as 0,1 ; can be cast to arbitrary num_rows
typedef matrix<double, 0, 1> sample_type;
typedef string label_type;

typedef std::vector<sample_type> v_sample_type;
typedef std::vector<label_type> v_label_type;


// any typedefs
typedef any_trainer<sample_type, label_type> a_tr;
typedef any_decision_function<sample_type, label_type> a_df;


// kernel typedefs
typedef histogram_intersection_kernel<sample_type> hist_kernel;
typedef linear_kernel<sample_type> lin_kernel;
typedef radial_basis_kernel<sample_type> rbf_kernel;
typedef polynomial_kernel<sample_type> poly_kernel;
typedef sigmoid_kernel<sample_type> sig_kernel;


// individual trainer typedefs
// one vs one trainer typedefs
typedef one_vs_one_trainer<any_trainer<sample_type>, label_type> ovo_trainer_type;
typedef one_vs_one_decision_function<ovo_trainer_type> ovo_trained_function_type;
typedef one_vs_one_decision_function<ovo_trainer_type, decision_function<rbf_kernel>> ovo_trained_function_type_rbf_df;

// one vs all trainer typedefs
typedef one_vs_all_trainer<any_trainer<sample_type>, label_type> ova_trainer_type;
typedef one_vs_all_decision_function<ova_trainer_type> ova_trained_function_type;
typedef one_vs_all_decision_function<ova_trainer_type, decision_function<rbf_kernel>> ova_trained_function_type_rbf_df;

// svm multiclass linear trainer typedefs
typedef svm_multiclass_linear_trainer<lin_kernel, label_type> svm_ml_trainer_type;
typedef multiclass_linear_decision_function<lin_kernel, label_type> svm_ml_trained_function_type;



/*
 *  TRAINER TEMPLATE
 */

//_______________________________________________________________________________________________________
class trainer_template {
 public:
  trainer_template() {}

  TrainerType getTrainerType() { return m_trainer_type; }
  TrainerName getTrainerName() { return m_trainer_name; }

  void setVerbosity(bool verbose) { m_verbose = verbose; }

  a_tr getTrainer() {
    if (m_trainer.is_empty()) {
      cerr << "Trainer not set!" << endl;
      exit(-1);
    }
    return m_trainer;
  }

  a_df train(const v_sample_type& all_samples, const v_label_type& all_labels) const {
    if (m_trainer.is_empty()) {
      cerr << "Trainer not set!" << endl;
      exit(-1);
    }
    return m_trainer.train(all_samples, all_labels);
  }

  virtual matrix<double> crossValidation(const v_sample_type& samples, const v_label_type& labels, const long folds) = 0;

 protected:
  void setTrainerType(TrainerType type) { m_trainer_type = type; }
  void setTrainerName(TrainerName name) { m_trainer_name = name; }

  a_tr m_trainer;
  bool m_verbose = false;

 private:
  TrainerType m_trainer_type = TrainerType::TEMPLATE;
  TrainerName m_trainer_name = TrainerName::TEMPLATE;

};


//template<typename T> trainer_template * createInstance() { return new T; }
//typedef std::map<std::string, trainer_template*(*)()> map_type;
//map_type type_map;
//type_map["ONE_VS_ONE"] = &createInstance<ovo_trainer>;



/*
 *   ### TRAINER DECLARATIONS ###
 */



/*
 *  ONE VS ONE
 */

//_______________________________________________________________________________________________________
class ovo_trainer : public trainer_template {
 public:
  typedef ovo_trainer_type T;

  ovo_trainer(bool verbose = false, int num_threads = 4, double kernel_gamma = 0.1) {
    setTrainerType(TrainerType::MULTICLASS);
    setTrainerName(TrainerName::ONE_VS_ONE);
    m_verbose = verbose;

    m_trainer.clear();
    m_trainer.get<T>();

    krr_trainer<rbf_kernel> krr_rbf_trainer;
    krr_rbf_trainer.set_kernel(rbf_kernel(kernel_gamma));
    m_trainer.cast_to<T>().set_trainer(krr_rbf_trainer);

    m_trainer.cast_to<T>().set_num_threads(num_threads);
    if (m_verbose)
      m_trainer.cast_to<T>().be_verbose();
  }

  matrix<double> crossValidation(const v_sample_type& samples, const v_label_type& labels, const long folds = 5) {
    if (m_trainer.is_empty()) {
      cerr << "Trainer not set!" << endl;
      exit(-1);
    }
    return cross_validate_multiclass_trainer(m_trainer.cast_to<T>(), samples, labels, folds);
  }

 private:
};



/*
 *  ONE VS ALL
 */

//_______________________________________________________________________________________________________
class ova_trainer : public trainer_template {
 public:
  typedef ova_trainer_type T;

  ova_trainer(bool verbose = false, int num_threads = 4, double kernel_gamma = 0.1) {
    setTrainerType(TrainerType::MULTICLASS);
    setTrainerName(TrainerName::ONE_VS_ALL);
    m_verbose = verbose;

    m_trainer.clear();
    m_trainer.get<T>();

    krr_trainer<rbf_kernel> krr_rbf_trainer;
    krr_rbf_trainer.set_kernel(rbf_kernel(kernel_gamma));
    m_trainer.cast_to<T>().set_trainer(krr_rbf_trainer);

    m_trainer.cast_to<T>().set_num_threads(num_threads);
    if (m_verbose)
      m_trainer.cast_to<T>().be_verbose();
  }

  matrix<double> crossValidation(const v_sample_type& samples, const v_label_type& labels, const long folds = 5) {
    if (m_trainer.is_empty()) {
      cerr << "Trainer not set!" << endl;
      exit(-1);
    }
    return cross_validate_multiclass_trainer(m_trainer.cast_to<T>(), samples, labels, folds);
  }

 private:
};



/*
 *  SVM MULTICLASS LINEAR
 */

//_______________________________________________________________________________________________________
class svm_ml_trainer : public trainer_template {
 public:
  typedef svm_ml_trainer_type T;

  svm_ml_trainer(bool verbose = false, int num_threads = 4) {
    setTrainerType(TrainerType::MULTICLASS);
    setTrainerName(TrainerName::SVM_MULTICLASS_LINEAR);
    m_verbose = verbose;

    m_trainer.clear();
    m_trainer.get<T>();

    m_trainer.cast_to<T>().set_num_threads(num_threads);
    if (m_verbose)
      m_trainer.cast_to<T>().be_verbose();
  }

  matrix<double> crossValidation(const v_sample_type& samples, const v_label_type& labels, const long folds = 5) {
    if (m_trainer.is_empty()) {
      cerr << "Trainer not set!" << endl;
      exit(-1);
    }
    return cross_validate_multiclass_trainer(m_trainer.cast_to<T>(), samples, labels, folds);
  }

 private:
};




#endif // _DLIB_TRAINERS_H_
