#include "libgrt_util.h"
#include "cmdline.h"
#include <stdint.h>
#include <math.h>

/* some helper functions */
int    push_back_if_not_there(string &label, vector<string> &labelset);
string centered(int, double);
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
  c.add         ("help",          'h', "print this message");
  c.add         ("no-confusion",  'c', "report confusion martix");
  c.add         ("no-precision",  'p', "report precision, per class and overall");
  c.add         ("no-recall",     'r', "report recall, per class and overall");
  c.add<double> ("F-score",       'F', "report F-beta score, beta defaults to 1", false, 1);
  c.footer      ("[filename] ...");

  /* parse the classifier-common arguments */
  if (!c.parse(argc,argv)) {
    cerr << c.usage() << endl << c.error() << endl;
    return -1;
  }

  if (c.exist("help")) {
    cerr << c.usage() << endl;
    return 0;
  }

  if ((c.exist("no-confusion") && c.exist("no-precision") && c.exist("no-recall") && c.get<double>("F-score")==0)) {
    cerr << c.usage() << endl << "error: need at least one reporting option" << endl;
    return -1;
  }

  /* handling of TERM and INT signal */
  set_running_indicator(&is_running);

  /* open standard input or file argument */
  istream &in = grt_fileinput(c);

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

  /* print confusion matrix */
  if (!c.exist("no-confusion")) {
    size_t tab_size = 0;
    for (auto label : labelset)
      tab_size = tab_size < label.size() ? label.size() : tab_size;

    /* print the header */
    cout << string(tab_size,' ');
    for (auto label : labelset) {
      cout << "  " << label << " ";
    }
    cout << endl;

    cout << string(tab_size,'-') << " ";
    for (auto label : labelset)
      cout << string(label.size()+2,'-') << " ";
    cout << endl;

    /* print the row */
    for (uint64_t i=0; i<labelset.size(); i++) {
      cout << labelset[i];
      cout << string(tab_size - labelset[i].size(), ' ');

      for(uint64_t j=0; j<labelset.size(); j++) {
        string num = to_string( (*confusion)[i][j] );
        int pre = (labelset[j].size() + 2 - num.size())/2,
            post = labelset[j].size() + 2 - num.size() - pre;

        if ((*confusion)[i][j] == 0)
          cout << " " << string(labelset[j].size() + 2, ' ');
        else
          cout << " " << string(pre, ' ') << num << string(post, ' ');
      }
      cout << endl;
    }

    cout << string(tab_size,'-') << " ";
    for (auto label : labelset)
      cout << string(label.size()+2,'-') << " ";
    cout << endl;

    cout << endl;
  }

  /* calculate scores and matrix */
  // see https://en.wikipedia.org/wiki/Precision_and_recall
  vector<uint64_t> TP = diag( *confusion );
  vector<uint64_t> FP = rowsum(*confusion) - TP;
  vector<uint64_t> TN = sum(*confusion) - colsum(*confusion);
  vector<uint64_t> FN = colsum(*confusion) - TP;
  double beta = c.get<double>("F-score");

  if (!c.exist("no-recall") || !c.exist("no-precision") || beta>0) {
    size_t tab_size = 2;
    for (auto label : labelset)
      tab_size = tab_size < label.size() ? label.size() : tab_size;
    tab_size += 1;

    vector<double> recall, precision, Fbeta;

    cout << "  " << string(tab_size - 2 + 1, ' ');
    if (!c.exist("no-recall"))    cout << "  recall  ";
    if (!c.exist("no-precision")) cout << "  precision  ";
    if (beta>0)                   cout << "  Fbeta  ";
    cout << endl;

    cout << string(tab_size, '-') << " " ;
    if (!c.exist("no-recall"))    cout << " -------- ";
    if (!c.exist("no-precision")) cout << " ----------- ";
    if (beta>0)                   cout << " ------- ";
    cout << endl;

    for (size_t i=0; i<labelset.size(); i++) {
      recall.push_back( TP[i] / (double) (TP[i] + FN[i]) );
      precision.push_back( TP[i] / (double) (TP[i] + FP[i]) );
      Fbeta.push_back( (1+beta*beta) * TP[i] / ((1+beta*beta) * TP[i] + beta*beta * FN[i] + FN[i]) );

      cout << labelset[i] << string(tab_size - labelset[i].size() + 1,' ');
      if (!c.exist("no-recall"))    cout << centered(sizeof("  recall "), recall.back() );
      if (!c.exist("no-precision")) cout << centered(sizeof("  precision "), precision.back() );
      if (beta>0)                   cout << centered(sizeof("  Fbeta "), Fbeta.back() );
      cout << endl;
    }

    cout << string(tab_size +1, ' ');
    if (!c.exist("no-recall"))    cout << centered(sizeof("  recall "), sum(recall)/recall.size());
    if (!c.exist("no-precision")) cout << centered(sizeof("  precision "), sum(precision)/recall.size());
    if (beta>0)                   cout << centered(sizeof("  Fbeta "), sum(Fbeta)/recall.size());
    cout << endl;
  }

  return 0;
}

string centered(int tab_size, double value) {
  stringstream ss;
  string val = to_string(value);
  if (tab_size < 5) tab_size = 5;
  if (val.size() > 5) val.resize(5);
  int pre = (tab_size - val.size()) / 2,
     post =  tab_size - val.size() - pre;

  ss << string(pre,' ') << val << string(post, ' ');
  return ss.str();
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

