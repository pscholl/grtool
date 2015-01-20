#include "libgrt_util.h"
#include "cmdline.h"
#include <stdint.h>

/* some helper functions */
int               push_back_if_not_there(string &label, vector<string> &labelset);
template< class T> vector<T>  diag(Matrix<T> &m);
template< class T> T          sum(vector<T> &m);
template< class T> T          sum(Matrix<T> &m);
template< class T> vector<T>  rowsum(Matrix<T> &m);
template< class T> vector<T>  colsum(Matrix<T> &m);
template< class T> Matrix<T>* resize_martix(Matrix<T> *old, T newsize);
template< class T> vector<T>  operator-(const std::vector<T> &a, const std::vector<T> &b);
template< class T> vector<T>  operator*(T a, const std::vector<T> &b);
template< class T> vector<T>  operator-(T a, const std::vector<T> &b);
template< class T> vector<T>  operator+(T a, const std::vector<T> &b);

int main(int argc, char *argv[])
{
  static bool is_running = true;
  cmdline::parser c;
  c.add         ("help",       'h', "print this message");
  c.add         ("confusion",  'c', "report confusion martix");
  c.add         ("precision",  'p', "report precision, per class and overall");
  c.add         ("recall",     'r', "report recall, per class and overall");
  c.add<double> ("F-score",    'F', "report F-beta score, beta defaults to 1", false, 0);
  c.footer      ("[filename] ...");

  /* parse the classifier-common arguments */
  if (!c.parse(argc,argv)) {
    cerr << c.usage() << "\n" << c.error() << "\n" ;
    return -1;
  }

  if (c.exist("help")) {
    cerr << c.usage() << endl;
    return 0;
  }

  /* handling of TERM and INT signal and set verbosity */
  set_running_indicator(&is_running);

  /* open standard input or file argument */
  string filename = c.rest().size() > 0 ? c.rest()[0] : "-";
  ifstream inf(filename);
  if (filename!="-" && !inf) {
    cerr << "unable to open file: " << filename << endl;
    return -1;
  }
  istream &in = filename != "-" ? inf : cin;
  string prediction, label;

  /* prepare the labelset and build confusion matrix */
  vector<string> labelset;
  Matrix<uint64_t> *confusion = new Matrix<uint64_t>();

  while (in >> prediction >> label)
  {
    int64_t idxA = push_back_if_not_there(prediction, labelset),
             idxB = push_back_if_not_there(label, labelset);

    if (confusion->getNumRows() != labelset.size())
      confusion = resize_martix(confusion, labelset.size());

    (*confusion)[idxA][idxB] += 1;
  }

  /* calculate scores and matrix */
  if (c.exist("confusion")) {
    for (auto label : labelset)
      cout << "\t" << label;
    cout << endl;

    for (uint64_t i=0; i<labelset.size(); i++) {
      cout << labelset[i];
      for(uint64_t j=0; j<labelset.size(); j++)
        cout << "\t" << (*confusion)[i][j];
      cout << endl;
    }
    cout << endl;
  }

  // see https://en.wikipedia.org/wiki/Precision_and_recall
  vector<uint64_t> TP = diag( *confusion );
  vector<uint64_t> FP = rowsum(*confusion) - TP;
  vector<uint64_t> TN = sum(*confusion) - colsum(*confusion);
  vector<uint64_t> FN = colsum(*confusion) - TP;
  double beta = c.get<double>("F-score");

  if (c.exist("recall") || c.exist("precision") || beta>0) {
    cout << "# class";
    if (c.exist("recall"))    cout << "\trecall";
    if (c.exist("precision")) cout << "\tprecision";
    if (beta>0)               cout << "\tFbeta";
    cout << endl;

    for (size_t i=0; i<labelset.size(); i++) {
      cout << labelset[i];
      if (c.exist("recall"))    cout << "\t" << TP[i] / (double) (TP[i] + FN[i]);
      if (c.exist("precision")) cout << "\t" << TP[i] / (double) (TP[i] + FP[i]);
      if (beta>0)               cout << "\t" << (1+beta*beta) * TP[i] / ((1+beta*beta) * TP[i] + beta*beta * FN[i] + FN[i]);
      cout << endl;
    }
    cout << endl;
  }

  return 0;
}

template< class T>
vector<T> diag(Matrix<T> &m) {
  vector<T> d;
  for (int i=0; i<m.getNumRows(); i++)
    d.push_back(m[i][i]);
  return d;
}

template< class T>
T sum(vector<T> &m) {
  T result = m[0];
  for (int i=1; i<m.size(); i++)
    result += m[i];
  return result;
}

template< class T>
T sum(Matrix<T> &m) {
  T result = m[0][0];
  for (int i=0; i<m.getNumRows(); i++)
    for (int j=1; j<m.getNumCols(); j++)
      result += m[i][j];
  return result;
}

template< class T>
Matrix<T>* resize_martix(Matrix<T> *old, T newsize) {
  Matrix<T> *confusion = new Matrix<T>(newsize, newsize);
  confusion->setAllValues(0);

  for (T i=0; i<old->getNumRows(); i++)
    for (T j=0; j<old->getNumRows(); j++)
      (*confusion)[i][j] = (*old)[i][j];

  delete old;
  return confusion;
}

template< class T>
vector<T>  rowsum(Matrix<T> &m) {
  vector<T> result;
  for (int i=0; i<m.getNumRows(); i++) {
    vector<T> row = m.getRowVector(i);
    result.push_back( sum(row) );
  }
  return result;
}

template< class T>
vector<T>  colsum(Matrix<T> &m) {
  vector<T> result;
  for (int i=0; i<m.getNumCols(); i++) {
    vector<T> col = m.getColVector(i);
    result.push_back( sum(col) );
  }
  return result;
}

int push_back_if_not_there(string &label, vector<string> &labelset)
{
 if( std::find(labelset.begin(), labelset.end(), label) == labelset.end() )
  labelset.push_back(label);

 return std::find(labelset.begin(), labelset.end(), label) - labelset.begin();
}

template< class T> 
std::vector<T> operator-(const std::vector<T> &a, const std::vector<T> &b)
{
  std::vector<T> res(a.size());
  for(size_t i=0; i<a.size(); ++i)
    res[i]=a[i]-b[i];
  return res;
}

template< class T> vector<T>  operator-(T a, const std::vector<T> &b)
{
  vector<T> res(b.size());
  for (size_t i=0; i<b.size(); ++i)
    res[i]=a-b[i];
  return res;
}

template< class T> vector<T>  operator*(T a, const std::vector<T> &b)
{
  vector<T> res(b.size());
  for (size_t i=0; i<b.size(); ++i)
    res[i]=a*b[i];
  return res;
}

template< class T> vector<T>  operator+(T a, const std::vector<T> &b)
{
  vector<T> res(b.size());
  for (size_t i=0; i<b.size(); ++i)
    res[i]=a+b[i];
  return res;
}

