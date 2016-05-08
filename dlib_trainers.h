/*
 * Author: Sebastian Boettcher
 *
 * Conveniance wrappers and tools for dlib trainers.
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
    ######## ##    ## ##     ## ##     ##  ######
    ##       ###   ## ##     ## ###   ### ##    ##
    ##       ####  ## ##     ## #### #### ##
    ######   ## ## ## ##     ## ## ### ##  ######
    ##       ##  #### ##     ## ##     ##       ##
    ##       ##   ### ##     ## ##     ## ##    ##
    ######## ##    ##  #######  ##     ##  ######
*/

BETTER_ENUM(TrainerType, int,
  TEMPLATE,
  BINARY,
  MULTICLASS,
  REGRESSION
)

BETTER_ENUM(TrainerName, int,
  TEMPLATE,
  // multiclass
  ONE_VS_ONE,
  ONE_VS_ALL,
  SVM_MULTICLASS_LINEAR,
  // binary
  RVM,
  SVM_C,
  SVM_C_LINEAR,
  SVM_C_LINEAR_DCD,
  SVM_C_EKM,
  SVM_NU,
  // regression
  KRR,
  RBF_NETWORK,
  RR,
  RVM_REG,
  SVR,
  SVR_LINEAR
)

multimap<TrainerType, TrainerName> type_name_map = {
  {TrainerType::MULTICLASS, TrainerName::ONE_VS_ONE},
  {TrainerType::MULTICLASS, TrainerName::ONE_VS_ALL},
  {TrainerType::MULTICLASS, TrainerName::SVM_MULTICLASS_LINEAR},
  {TrainerType::BINARY, TrainerName::RVM},
  {TrainerType::BINARY, TrainerName::SVM_C},
  {TrainerType::BINARY, TrainerName::SVM_C_LINEAR},
  {TrainerType::BINARY, TrainerName::SVM_C_LINEAR_DCD},
  {TrainerType::BINARY, TrainerName::SVM_C_EKM},
  {TrainerType::BINARY, TrainerName::SVM_NU},
  {TrainerType::REGRESSION, TrainerName::KRR},
  {TrainerType::REGRESSION, TrainerName::RBF_NETWORK},
  {TrainerType::REGRESSION, TrainerName::RR},
  {TrainerType::REGRESSION, TrainerName::RVM_REG},
  {TrainerType::REGRESSION, TrainerName::SVR},
  {TrainerType::REGRESSION, TrainerName::SVR_LINEAR}
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

bool operator==(const std::string& name_str, const TrainerName& name) {
  if (!TrainerName::_is_valid_nocase(name_str.c_str()))
    return false;
  return TrainerName::_from_string_nocase(name_str.c_str()) == name;
}

bool classifierExists(string name) {
  for (TrainerName tname : TrainerName::_values())
    if (name == tname)
      return true;
  return false;
}

bool classifierIsType(string name, TrainerType type) {
  if (classifierExists(name))
    for (auto it = type_name_map.equal_range(type).first; it != type_name_map.equal_range(type).second; ++it)
      if (name == it->second)
        return true;
  return false;
}

std::vector<string> classifierGetType(TrainerType type) {
  std::vector<string> ret;
  for (auto it = type_name_map.equal_range(type).first; it != type_name_map.equal_range(type).second; ++it)
    ret.push_back(it->second._to_string());
  return ret;
}


/*
    ######## ##    ## ########  ######## ########  ######## ########  ######
       ##     ##  ##  ##     ## ##       ##     ## ##       ##       ##    ##
       ##      ####   ##     ## ##       ##     ## ##       ##       ##
       ##       ##    ########  ######   ##     ## ######   ######    ######
       ##       ##    ##        ##       ##     ## ##       ##             ##
       ##       ##    ##        ##       ##     ## ##       ##       ##    ##
       ##       ##    ##        ######## ########  ######## ##        ######
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

#define KERNEL_TYPE "list", "hist", "lin", "rbf", "poly", "sig"


// individual trainer typedefs

// one vs one trainer typedefs
typedef one_vs_one_trainer<any_trainer<sample_type>, label_type> ovo_trainer_type;
typedef one_vs_one_decision_function<ovo_trainer_type> ovo_trained_function_type;
typedef one_vs_one_decision_function<ovo_trainer_type, decision_function<offset_kernel<hist_kernel>>> ovo_trained_function_type_hist_df;
typedef one_vs_one_decision_function<ovo_trainer_type, decision_function<offset_kernel<lin_kernel>>> ovo_trained_function_type_lin_df;
typedef one_vs_one_decision_function<ovo_trainer_type, decision_function<lin_kernel>> ovo_trained_function_type_lin_no_df;
typedef one_vs_one_decision_function<ovo_trainer_type, decision_function<offset_kernel<rbf_kernel>>> ovo_trained_function_type_rbf_df;
typedef one_vs_one_decision_function<ovo_trainer_type, decision_function<offset_kernel<poly_kernel>>> ovo_trained_function_type_poly_df;
typedef one_vs_one_decision_function<ovo_trainer_type, decision_function<offset_kernel<sig_kernel>>> ovo_trained_function_type_sig_df;

// one vs all trainer typedefs
typedef one_vs_all_trainer<any_trainer<sample_type>, label_type> ova_trainer_type;
typedef one_vs_all_decision_function<ova_trainer_type> ova_trained_function_type;
typedef one_vs_all_decision_function<ova_trainer_type, decision_function<offset_kernel<hist_kernel>>> ova_trained_function_type_hist_df;
typedef one_vs_all_decision_function<ova_trainer_type, decision_function<offset_kernel<lin_kernel>>> ova_trained_function_type_lin_df;
typedef one_vs_all_decision_function<ova_trainer_type, decision_function<lin_kernel>> ova_trained_function_type_lin_no_df;
typedef one_vs_all_decision_function<ova_trainer_type, decision_function<offset_kernel<rbf_kernel>>> ova_trained_function_type_rbf_df;
typedef one_vs_all_decision_function<ova_trainer_type, decision_function<offset_kernel<poly_kernel>>> ova_trained_function_type_poly_df;
typedef one_vs_all_decision_function<ova_trainer_type, decision_function<offset_kernel<sig_kernel>>> ova_trained_function_type_sig_df;

// svm multiclass linear trainer typedefs
typedef svm_multiclass_linear_trainer<lin_kernel, label_type> svm_ml_trainer_type;
typedef multiclass_linear_decision_function<lin_kernel, label_type> svm_ml_trained_function_type;



/*
    ######## ######## ##     ## ########  ##          ###    ######## ########
       ##    ##       ###   ### ##     ## ##         ## ##      ##    ##
       ##    ##       #### #### ##     ## ##        ##   ##     ##    ##
       ##    ######   ## ### ## ########  ##       ##     ##    ##    ######
       ##    ##       ##     ## ##        ##       #########    ##    ##
       ##    ##       ##     ## ##        ##       ##     ##    ##    ##
       ##    ######## ##     ## ##        ######## ##     ##    ##    ########
*/

//_______________________________________________________________________________________________________
class trainer_template {
 public:
  trainer_template() {}

  TrainerType getTrainerType() { return m_trainer_type; }
  TrainerName getTrainerName() { return m_trainer_name; }

  void setVerbosity(bool verbose) { m_verbose = verbose; }

  string getKernel() { return m_kernel; }

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
  string m_kernel = "n/a";

 private:
  TrainerType m_trainer_type = TrainerType::TEMPLATE;
  TrainerName m_trainer_name = TrainerName::TEMPLATE;

};




/*
 *   ### TRAINER DECLARATIONS ###
 */


/*
 #######  ##    ## ########    ##     ##  ######      #######  ##    ## ########
##     ## ###   ## ##          ##     ## ##    ##    ##     ## ###   ## ##
##     ## ####  ## ##          ##     ## ##          ##     ## ####  ## ##
##     ## ## ## ## ######      ##     ##  ######     ##     ## ## ## ## ######
##     ## ##  #### ##           ##   ##        ##    ##     ## ##  #### ##
##     ## ##   ### ##            ## ##   ##    ##    ##     ## ##   ### ##
 #######  ##    ## ########       ###     ######      #######  ##    ## ########
*/

//_______________________________________________________________________________________________________
class ovo_trainer : public trainer_template {
 public:
  typedef ovo_trainer_type T;

  ovo_trainer(bool verbose = false, int num_threads = 4, string kernel = "", any_trainer<sample_type> bin_tr = krr_trainer<rbf_kernel>()) {
    setTrainerType(TrainerType::MULTICLASS);
    setTrainerName(TrainerName::ONE_VS_ONE);
    m_verbose = verbose;
    m_kernel = kernel;

    m_trainer.clear();
    m_trainer.get<T>();

    m_trainer.cast_to<T>().set_trainer(bin_tr);

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
};



/*
 #######  ##    ## ########    ##     ##  ######        ###    ##       ##
##     ## ###   ## ##          ##     ## ##    ##      ## ##   ##       ##
##     ## ####  ## ##          ##     ## ##           ##   ##  ##       ##
##     ## ## ## ## ######      ##     ##  ######     ##     ## ##       ##
##     ## ##  #### ##           ##   ##        ##    ######### ##       ##
##     ## ##   ### ##            ## ##   ##    ##    ##     ## ##       ##
 #######  ##    ## ########       ###     ######     ##     ## ######## ########
*/

//_______________________________________________________________________________________________________
class ova_trainer : public trainer_template {
 public:
  typedef ova_trainer_type T;

  ova_trainer(bool verbose = false, int num_threads = 4, string kernel = "", any_trainer<sample_type> bin_tr = krr_trainer<rbf_kernel>()) {
    setTrainerType(TrainerType::MULTICLASS);
    setTrainerName(TrainerName::ONE_VS_ALL);
    m_verbose = verbose;
    m_kernel = kernel;

    m_trainer.clear();
    m_trainer.get<T>();

    m_trainer.cast_to<T>().set_trainer(bin_tr);

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
};



/*
 ######  ##     ## ##     ##    ##       #### ##    ## ########    ###    ########
##    ## ##     ## ###   ###    ##        ##  ###   ## ##         ## ##   ##     ##
##       ##     ## #### ####    ##        ##  ####  ## ##        ##   ##  ##     ##
 ######  ##     ## ## ### ##    ##        ##  ## ## ## ######   ##     ## ########
      ##  ##   ##  ##     ##    ##        ##  ##  #### ##       ######### ##   ##
##    ##   ## ##   ##     ##    ##        ##  ##   ### ##       ##     ## ##    ##
 ######     ###    ##     ##    ######## #### ##    ## ######## ##     ## ##     ##
*/

//_______________________________________________________________________________________________________
class svm_ml_trainer : public trainer_template {
 public:
  typedef svm_ml_trainer_type T;

  svm_ml_trainer(bool verbose = false, int num_threads = 4, bool nonneg = false, double epsilon = 0.001, int iterations = 10000, double regularization = 1) {
    setTrainerType(TrainerType::MULTICLASS);
    setTrainerName(TrainerName::SVM_MULTICLASS_LINEAR);
    m_verbose = verbose;

    m_trainer.clear();
    m_trainer.get<T>();

    m_trainer.cast_to<T>().set_learns_nonnegative_weights(nonneg);
    m_trainer.cast_to<T>().set_epsilon(epsilon);
    m_trainer.cast_to<T>().set_max_iterations(iterations);
    m_trainer.cast_to<T>().set_c(regularization);

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
};




#endif // _DLIB_TRAINERS_H_
