#include "libgrt_util.h"
#include "cmdline.h"
#include <stdint.h>
#include <math.h>

class Group {
  public:
  Matrix<uint64_t> *confusion;
  vector<string> labelset;
  vector<string> lines;

  void calculate_score(double beta);
  double get_meanscore(string);

  vector< uint64_t > TP,TN,FP,FN;
  vector< double >   Fbeta,recall,precision;

  string to_string(cmdline::parser &c);
};

/* some helper functions */
int    push_back_if_not_there(string &label, vector<string> &labelset);
string centered(int, string);
string centered(int, double);
string meanstd(vector< double >);
template< class T> vector<T>  diag(Matrix<T> &m);
template< class T> T          sum(vector<T> m);
template< class T> T          sum(Matrix<T> &m);
template< class T> vector<T>  abs(vector<T> m);
template< class T> vector<T>  pow(vector<T> m, double pow);
template< class T> vector<T>  rowsum(Matrix<T> &m);
template< class T> vector<T>  colsum(Matrix<T> &m);
template< class T> Matrix<T>* resize_matrix(Matrix<T> *old, T newsize);
template< class T> vector<T>  operator-(const std::vector<T> &a, const std::vector<T> &b);
template< class T> vector<T>  operator*(T a, const std::vector<T> &b);
template< class T> vector<T>  operator-(T a, const std::vector<T> &b);
template< class T> vector<T>  operator-(const std::vector<T> &b, T a);
template< class T> vector<T>  operator+(T a, const std::vector<T> &b);

int main(int argc, char *argv[])
{
  static bool is_running = true;
  cmdline::parser c;
  c.add         ("help",          'h', "print this message");
  c.add         ("no-confusion",  'c', "report confusion matrix");
  c.add         ("no-precision",  'p', "report precision, per class and overall");
  c.add         ("no-recall",     'r', "report recall, per class and overall");
  c.add<double> ("F-score",       'F', "report F-beta score, beta defaults to 1, 0 to disable", false, 1);
  c.add<int>    ("group",         'g', "assemble N repetitions (divided by empty lines) into one result (default is to assemble all)", false, 0);
  c.add<string> ("select-by",     's', "select output by ascending mean [F1,recall,precision,disabled] score (disabled per default)", false, "disabled", cmdline::oneof<string>("F1","recall","precision","disabled"));
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

  /* open standard input or file argument */
  istream &in = grt_fileinput(c);
  if (!in) return -1;

  /* we read in N-groups divided by empty lines */
  Group *g = new Group(); string line; double selector_score=0; uint64_t num_groups=0;
  while(getline(in,line)) {
    if (line=="") {
      if (g->lines.size() == 0)
        continue;
      else if (c.get<int>("group") == 0 || num_groups < c.get<int>("group"))
        continue;
      else {
        g->calculate_score(c.get<double>("F-score"));
        double score = g->get_meanscore(c.get<string>("select-by"));

        if (selector_score <= score) {
          selector_score = score;
          cout << g->to_string(c);
        }
      }
    } else {
      num_groups += g->lines.size() == 0;
      g->lines.push_back(line);
    }
  }

  if (g->lines.size() > 0) {
    g->calculate_score(c.get<double>("F-score"));
    double score = g->get_meanscore(c.get<string>("select-by"));

    if (selector_score <= score) {
      selector_score = score;
      cout << g->to_string(c);
    }
  }

  return 0;
}

void Group::calculate_score(double beta)
{
  confusion = new Matrix<uint64_t>();

  for (auto line : lines)
  {
    stringstream ss(line); string prediction, label;
    ss >> prediction >> label;

    int64_t idxA = push_back_if_not_there(prediction, labelset),
            idxB = push_back_if_not_there(label,      labelset);

    if (confusion->getNumRows() != labelset.size())
      confusion = resize_matrix(confusion, labelset.size());

    (*confusion)[idxA][idxB] += 1;
  }

  // see https://en.wikipedia.org/wiki/Precision_and_recall
  vector<uint64_t> TP = diag( *confusion );
  vector<uint64_t> FP = rowsum(*confusion) - TP;
  vector<uint64_t> TN = sum(*confusion) - colsum(*confusion);
  vector<uint64_t> FN = colsum(*confusion) - TP;

  for (size_t i=0; i<labelset.size(); i++) {
    recall.push_back( TP[i] / (double) (TP[i] + FN[i]) );
    precision.push_back( TP[i] / (double) (TP[i] + FP[i]) );
    Fbeta.push_back( (1+beta*beta) * TP[i] / ((1+beta*beta) * TP[i] + beta*beta * FN[i] + FN[i]) );
  }
}

double Group::get_meanscore(string which)
{
  vector<double> *score, nonan;

  if (which.find("none") != string::npos)           return 0;
  else if (which.find("F1") != string::npos)        score = &Fbeta;
  else if (which.find("recall") != string::npos)    score = &recall;
  else if (which.find("precision") != string::npos) score = &precision;
  else return 0;

  for (auto val : *score)
    if (!std::isnan(val))
      nonan.push_back(val);

  return sum(nonan)/nonan.size();
}

string Group::to_string(cmdline::parser &c) {
  stringstream cout;

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
        string num = std::to_string( (*confusion)[i][j] );
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

  double beta = c.get<double>("F-score");
  if (!c.exist("no-recall") || !c.exist("no-precision") || beta>0) {
    size_t tab_size = 2;
    for (auto label : labelset)
      tab_size = tab_size < label.size() ? label.size() : tab_size;
    tab_size += 1;

    uint64_t TAB_SIZE = 18;

    cout << "  " << string(tab_size - 2 + 1, ' ');
    if (!c.exist("no-recall"))    cout << centered(TAB_SIZE,"  recall  ");
    if (!c.exist("no-precision")) cout << centered(TAB_SIZE,"  precision  ");
    if (beta>0)                   cout << centered(TAB_SIZE,"  Fbeta  ");
    cout << endl;

    cout << string(tab_size, '-') << " " ;
    if (!c.exist("no-recall"))    cout << string(TAB_SIZE-1, '-') << " ";
    if (!c.exist("no-precision")) cout << string(TAB_SIZE-1, '-') << " ";
    if (beta>0)                   cout << string(TAB_SIZE-1, '-');
    cout << endl;

    for (size_t i=0; i<labelset.size(); i++) {
      cout << labelset[i] << string(tab_size - labelset[i].size() + 1,' ');
      if (!c.exist("no-recall"))    cout << centered(TAB_SIZE, std::isnan(recall.back())     ? "" : std::to_string(recall.back()));
      if (!c.exist("no-precision")) cout << centered(TAB_SIZE, std::isnan(precision.back()) ? "" : std::to_string(precision.back()));
      if (beta>0)                   cout << centered(TAB_SIZE, std::isnan(Fbeta.back())     ? "" : std::to_string(Fbeta.back()));
      cout << endl;
    }

    cout << string(tab_size +1, ' ');
    if (!c.exist("no-recall"))      cout << centered(TAB_SIZE, meanstd(recall));
    if (!c.exist("no-precision"))   cout << centered(TAB_SIZE, meanstd(precision));
    if (beta>0)                     cout << centered(TAB_SIZE, meanstd(Fbeta));
    cout << endl;
  }

  cout << endl;
  return cout.str();
}

string centered(int tab_size, string val) {
  stringstream ss;
  if (tab_size < 5) tab_size = 5;
  if (val.size() > tab_size-4) val.resize(tab_size-4);
  //if (val.size() > 5) val.resize(5);
  int pre = (tab_size - val.size()) / 2,
     post =  tab_size - val.size() - pre;

  ss << string(pre,' ') << val << string(post, ' ');
  return ss.str();
}

string centered(int tab_size, double value) {
  return centered(tab_size, to_string(value));
}

template< class T>
vector<T> diag(Matrix<T> &m) {
  vector<T> d;
  for (int i=0; i<m.getNumRows(); i++)
    d.push_back(m[i][i]);
  return d;
}

template< class T>
T sum(vector<T> m) {
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
Matrix<T>* resize_matrix(Matrix<T> *old, T newsize) {
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

template< class T>
vector<T> abs(vector<T> m) {
  vector<T> result;
  for (int i=0; i<m.size(); i++) {
    T val = m[i];
    result.push_back( std::abs(val) );
  }
  return result;
}

template< class T>
vector<T>  pow(vector<T> m, double pow) {
  vector<T> result;
  for (int i=0; i<m.size(); i++) {
    T val = std::pow(m[i], pow);
    result.push_back( val );
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

template< class T> vector<T>  operator-(const std::vector<T> &b, T a)
{
  vector<T> res(b.size());
  for (size_t i=0; i<b.size(); ++i)
    res[i]=b[i]-a;
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

string meanstd(vector< double > list) {
  vector< double > cleaned;

  for ( auto val : list )
    if (!std::isnan(val))
      cleaned.push_back(val);

  if (cleaned.size() == 0)
    return "";

  double mean = sum(cleaned)/cleaned.size();

  if (cleaned.size() == 1)
    return to_string(mean);

  stringstream ss;
  ss << mean << "/" << sqrt( sum(pow(abs(cleaned-mean),2))/cleaned.size() );
  return ss.str();
}
