#include "libgrt_util.h"
#include "cmdline.h"
#include <stdint.h>
#include <math.h>
#include <unordered_map>
#include <regex>
#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>

class Group {
  public:
  Matrix<uint64_t> *confusion = NULL;
  vector<string> labelset;
  vector<string> lines;

  void add_prediction(string, string);
  void calculate_score(double beta);
  void calculate_ead();
  double get_meanscore(string, double);

  vector< uint64_t > TP,TN,FP,FN;
  vector< double >   Fbeta,recall,precision,TNR,NPV;

  string to_string(cmdline::parser&, string tag);
  string to_flat_string(cmdline::parser&, string tag);

  string last_label = "NULL", last_prediction = "NULL";

  struct { // EAD errors according to Ward et.al. 2011
    uint64_t deletions = 0,
             ev_fragmented = 0,
             ev_fragmerged = 0,
             ev_merged = 0,
             correct = 0,
             re_merged = 0,
             re_fragmerged = 0,
             re_fragmented = 0,
             insertions = 0;
  } ead;

  uint64_t groundtruth_changed = 0,
           prediction_changed  = 0,
           total_frames        = 0;
};

/* some helper functions */
bool   value_differs(map<double,string>&, map<double,string>&);
int    push_back_if_not_there(string &label, vector<string> &labelset);
string centered(int, string, int DEFAULT=5);
string centered(int, double, int DEFAULT=5);
string centered(int, uint64_t, int DEFAULT=5);
string meanstd(vector< double >);
string mean(vector< double >);
template< class T> vector<T>  diag(Matrix<T> &m);
template< class T> T          sum(vector<T> m);
template< class T> T          sum(Matrix<T> &m);
template< class T> vector<T>  abs(vector<T> m);
template< class T> vector<T>  pow(vector<T> m, double pow);
template< class T> vector<T>  rowsum(Matrix<T> &m);
template< class T> vector<T>  colsum(Matrix<T> &m);
template< class T> Matrix<T>* resize_matrix(Matrix<T> *old, T newsize);
template< class T> vector<T>  operator-(const std::vector<T> &a, const std::vector<T> &b);
template< class T> vector<T>  operator+(const std::vector<T> &a, const std::vector<T> &b);
template< class T> vector<T>  operator*(T a, const std::vector<T> &b);
template< class T> vector<T>  operator-(T a, const std::vector<T> &b);
template< class T> vector<T>  operator-(const std::vector<T> &b, T a);
template< class T> vector<T>  operator+(T a, const std::vector<T> &b);

int main(int argc, char *argv[])
{
  static bool is_running = true;
  cmdline::parser c;
  c.add         ("help",          'h', "print this message");
  c.add         ("flat",          'f', "line-based output, implies --no-confusion");
  c.add         ("no-confusion",  'c', "report confusion matrix");
  c.add         ("no-score",      'n', "report recall, per class and overall");
  c.add         ("no-ead",        'e', "report event-analysis diagram");
  c.add<double> ("F-score",       'F', "beta value for F-score, default: 1", false, 1);
  c.add         ("group",         'g', "aggregate input lines by tags, a tag is a string enclosed in paranthesis");
  c.add         ("quiet",         'q', "print no warnings");
  c.add<string> ("sort",          's', "prints results in ascending mean [Fbeta,recall,precision,disabled] order", false, "disabled", cmdline::oneof<string>("Fbeta","recall","precision","disabled"));
  c.add         ("intermediate",  'i', "do not report intermediate scores");
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

  if (c.exist("sort") && !c.exist("group")) {
    cerr << c.usage() << endl << "erro: sorting can only be used in conjuction with tagged (-g) input" << endl;
    return -1;
  }

  if (c.exist("flat"))
    c.set_option("no-confusion");

  if (c.exist("no-score") && c.exist("no-confusion") && c.exist("no-ead")) {
    cerr << c.usage() << endl << "error: --no-confusion, --no-score and --no-ead can not be given at the same time" << endl;
    return -1;
  }

  /* open standard input or file argument */
  istream &in = grt_fileinput(c);
  if (!in) return -1;

  /* read multiple groups divided by tagged lines, if advised to do so.
   * Otherwise just read everything and print one report at the end.
   *
   * If grouping is enabled (by -g) and no selection option (-s) is given, all
   * groups will be reported after completly reading the input. Otherwise
   * whenever a new maximum on the selection is achieved.
   *
   * Tagged lines look like this:
   *  (tag) label prediction
   * and untagged ones:
   *  label prediction
   */
  string top_score_type = c.get<string>("sort"), top_tag = "";
  double top_score = .0, beta = c.get<double>("F-score");
  unordered_map<string,Group> groups;
           map<double,string> scores;
  string line, tag="None", prediction, label;
  regex tagged ("\\((.+)\\)\\s+(\\w+)\\s+(\\w+)\\s*.*"),
      untagged ("(\\w+)\\s+(\\w+)\\s*.*");
  smatch match;

  while (getline(in,line)) {
    if (trim(line)=="" || trim(line)[0]=='#') continue;

    if (c.exist("group")) {
      if (!regex_match(line,match,tagged)) {
        if (!c.exist("quiet")) cerr << line << " ignored (no tag?)" << endl;
        continue;
      }

      tag = match[1];
      label = match[2];
      prediction = match[3];
    } else {
      if (!regex_match(line,match,untagged)) {
        cerr << line << " ignored" << endl;
        continue;
      }

      label = match[1];
      prediction = match[2];
    }

    /* intermediate top-score reports */
    if (top_score_type != "disabled" && &in==&cin && c.exist("intermediate")) {
      double score;

      Group &g = groups[tag];
      score = g.get_meanscore(top_score_type,beta);

      if(scores.count(score) && scores[score] == tag)
        scores.erase(score);

      g.add_prediction(label, prediction);
      score = g.get_meanscore(top_score_type,beta);
      scores[score] = tag;

      if (!c.exist("flat"))
        for(auto &x : scores)
          cout << groups[x.second].to_string(c,x.second) << endl;
      else {
        // TODO
        // for(auto &x : scores)
        //   cout << groups[x.second].to_string(c,x.second);
        // cout << endl;
      }
    } else
      groups[tag].add_prediction(label, prediction);
  }

  if (top_score_type != "disabled" && groups.size() > 0) {
    if (c.exist("intermediate"))
        cout << "Final Top-Score (" << c.get<string>("sort") << "):" << endl;

    for (auto &x : groups)
      scores[x.second.get_meanscore(top_score_type,beta)] = x.first;

    int i=0;
    for (auto &x : scores)
      cout << (i++ != 0 ? "\n" : "") << groups[x.second].to_string(c,x.second);
  }
  else {
    int i=0;
    for (auto &group : groups)
      cout << (i++ != 0 ? "\n" : "") << (c.exist("flat") ?
              group.second.to_flat_string(c,group.first) :
                   group.second.to_string(c,group.first));
  }

  return 0;
}

void Group::add_prediction(string label, string prediction)
{
  /* first we calculate your every-day confusion matrix, which
   * is later used to calculate TP,TN,FN,FP scores and their stats */
  if (confusion == NULL)
    confusion = new Matrix<uint64_t>();

  int64_t idxA = push_back_if_not_there(prediction, labelset),
          idxB = push_back_if_not_there(label,      labelset);

  if (confusion->getNumRows() != labelset.size())
    confusion = resize_matrix(confusion, labelset.size());

  (*confusion)[idxA][idxB] += 1;

  /* events are hit when both labels are NULL, with one exception handled
   * when a double-NULL was encountered */
  if ( (last_label!=label && last_prediction!=prediction) ||
       (label=="NULL" && prediction=="NULL") ) {
    calculate_ead();
    prediction_changed = groundtruth_changed = 0;
    last_prediction = last_label = "NULL";
  }

  /* and then we also calculate the more in-depth analysis of Ward et.al.
   * - Performance Metrics for Activity Recognition.
   *
   * For this we need to keep track of the next and last label to score, 
   * one call before this one. */
  groundtruth_changed += label!=last_label;
  prediction_changed  += prediction!=last_prediction;

  // DEBUG
  // cerr << last_label << "\t" << label << "\t\t";
  // cerr << last_prediction << "\t" << prediction << "\t\t";
  // cerr << groundtruth_changed << "\t" << prediction_changed << endl;

  /* compress label sequences into one last_label */
  last_label      = label;
  last_prediction = prediction;
}

void Group::calculate_ead()
{
  if ( prediction_changed==0 && groundtruth_changed==0 )
    return;

  groundtruth_changed -= floor(groundtruth_changed/2);
  prediction_changed  -= floor(prediction_changed/2);

  // DEBUG
  // cerr << groundtruth_changed << "\t" << prediction_changed << endl;

  /* now for each of the error cases */
  if (prediction_changed == 0 && groundtruth_changed == 1) { // deletion
    ead.deletions++;
  } else if ( groundtruth_changed == 0 && prediction_changed == 1) {
    ead.insertions++;
  } else if ( groundtruth_changed == 1 && prediction_changed < 2 ) {
    ead.correct++;
  } else if ( groundtruth_changed == 1 && prediction_changed > 1 ) {
    ead.ev_fragmented += groundtruth_changed;
    ead.re_fragmented += prediction_changed;
  } else if ( groundtruth_changed > 1 && prediction_changed == 1) {
    ead.ev_merged += groundtruth_changed;
    ead.re_merged += prediction_changed;
  } else if ( groundtruth_changed > 1 && prediction_changed > 1) {
    ead.ev_fragmerged += groundtruth_changed;
    ead.re_fragmerged += prediction_changed;
  } else
    cerr << "this never happened" << endl;
}

void Group::calculate_score(double beta)
{
  if (confusion == NULL) return;

  // see https://en.wikipedia.org/wiki/Precision_and_recall
  vector<uint64_t> TP = diag(*confusion);
  vector<uint64_t> FP = rowsum(*confusion) - TP;
  vector<uint64_t> TN = sum(*confusion) - colsum(*confusion) - rowsum(*confusion) + TP;
  vector<uint64_t> FN = colsum(*confusion) - TP;

  recall.clear(); precision.clear(); Fbeta.clear();
  for (size_t i=0; i<labelset.size(); i++) {
    recall.push_back( TP[i] / (double) (TP[i] + FN[i]) );
    precision.push_back( TP[i] / (double) (TP[i] + FP[i]) );
    NPV.push_back( TN[i] / (double) (FN[i] + TN[i]) );
    TNR.push_back( TN[i] / (double) (TN[i] + FP[i]) );
    Fbeta.push_back( (1+pow(beta,2)) * (precision[i] * recall[i])/(pow(beta,2)*precision[i] + recall[i]) );
  }

  /* TODO prior to printing anythign we re-calc, this may be wrong here */
  //add_prediction("NULL","NULL");
  calculate_ead();
}

double Group::get_meanscore(string which, double beta)
{
  vector<double> *score, nonan;
  calculate_score(beta);

  if (which.find("none") != string::npos)           return 0;
  else if (which.find("Fbeta") != string::npos)     score = &Fbeta;
  else if (which.find("recall") != string::npos)    score = &recall;
  else if (which.find("precision") != string::npos) score = &precision;
  else if (which.find("NPV") != string::npos)       score = &NPV;
  else if (which.find("TNR") != string::npos)       score = &TNR;
  else return 0;

  for (auto val : *score)
    if (!std::isnan(val))
      nonan.push_back(val);
    else
      nonan.push_back(0.);

  return nonan.size()==0 ? 0. : sum(nonan)/nonan.size();
}

string Group::to_flat_string(cmdline::parser &c, string tag) {
  stringstream ss;
  calculate_score(c.get<double>("F-score"));

  if (c.exist("no-score"))
    return "";

  /* print out comment attached to this group if any */
  for( auto line : lines )
    if ( line[0] == '#' )
      cout << line << endl;

  /* print the header */
  double beta = c.get<double>("F-score");
  ss << "# ";
  ss << " groupname ";
  ss << "total_recall ";
  ss << "total_precision ";
  ss << "total_Fbeta ";
  ss << "total_NPV ";
  ss << "total_TNR ";
  for (auto label : labelset) {
    ss << label << "_recall ";
    ss << label << "_precision ";
    ss << label << "_Fbeta ";
    ss << label << "_NPV ";
    ss << label << "_TNR ";
  } ss << endl;

  /* print out the total scores first */
  ss << tag << " ";
  ss << mean(recall) << " ";
  ss << mean(precision) << " ";
  ss << mean(Fbeta) << " ";
  ss << mean(NPV) << " ";
  ss << mean(TNR) << " ";

  for (size_t i=0; i<labelset.size(); i++) {
    ss << (std::isnan(recall[i])     ? "0" : std::to_string(recall[i])) << " ";
    ss << (std::isnan(precision[i])  ? "0" : std::to_string(precision[i])) << " ";
    ss << (std::isnan(Fbeta[i])      ? "0" : std::to_string(Fbeta[i])) << " ";
    ss << (std::isnan(NPV[i])        ? "0" : std::to_string(Fbeta[i])) << " ";
    ss << (std::isnan(TNR[i])        ? "0" : std::to_string(Fbeta[i])) << " ";
  } ss << endl;

  return ss.str();

   // TODO also add the EAD
}

string Group::to_string(cmdline::parser &c, string tag) {
  stringstream cout;
  calculate_score(c.get<double>("F-score"));

  /* print out comment attached to this group if any */
  for( auto line : lines )
    if ( line[0] == '#' )
      cout << line << endl;

  /* print confusion matrix */
  if (!c.exist("no-confusion")) {
    size_t tab_size = 0;
    for (auto label : labelset)
      tab_size = tab_size < label.size() ? label.size() : tab_size;
    tab_size = tab_size < tag.size() ? tag.size() : tab_size;
    tab_size += 1;

    /* print the header */
    cout << tag << string(tab_size - tag.size(), ' ');
    for (auto label : labelset)
      cout << "  " << label << " ";
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
        int pre  = (labelset[j].size() + 2 - num.size())/2,
            post = labelset[j].size() + 2 - num.size() - pre;
        pre = pre < 0 ? 0 : pre;
        post = post < 0 ? 0 : post;

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
  }

  /* print stats */
  if (!c.exist("no-score")) {
    size_t tab_size = 2;
    double beta = c.get<double>("F-score");

    if (!c.exist("no-confusion"))
      cout << endl;

    for (auto label : labelset)
      tab_size = tab_size < label.size() ? label.size() : tab_size;
    tab_size = tab_size < tag.size() ? tag.size() : tab_size;
    tab_size += 1;

    uint64_t TAB_SIZE = 18;

    cout << tag << string(tab_size - tag.size(), ' ') << " ";
    cout << centered(TAB_SIZE,"  recall  ");
    cout << centered(TAB_SIZE,"  precision  ");
    cout << centered(TAB_SIZE,"  Fbeta  ");
    cout << centered(TAB_SIZE,"   NPV   ");
    cout << centered(TAB_SIZE,"   TNR   ");
    cout << endl;

    cout << string(tab_size, '-') << " " ;
    cout << string(TAB_SIZE-1, '-') << " ";
    cout << string(TAB_SIZE-1, '-') << " ";
    cout << string(TAB_SIZE-1, '-') << " ";
    cout << string(TAB_SIZE-1, '-') << " ";
    cout << string(TAB_SIZE-1, '-');
    cout << endl;

    for (size_t i=0; i<labelset.size(); i++) {
      cout << labelset[i] << string(tab_size - labelset[i].size() + 1,' ');
      cout << centered(TAB_SIZE, std::isnan(recall[i])    ? "" : std::to_string(recall[i]));
      cout << centered(TAB_SIZE, std::isnan(precision[i]) ? "" : std::to_string(precision[i]));
      cout << centered(TAB_SIZE, std::isnan(Fbeta[i])     ? "" : std::to_string(Fbeta[i]));
      cout << centered(TAB_SIZE, std::isnan(NPV[i])       ? "" : std::to_string(NPV[i]));
      cout << centered(TAB_SIZE, std::isnan(TNR[i])       ? "" : std::to_string(TNR[i]));
      cout << endl;
    }

    cout << string(tab_size+1, ' ');
    cout << centered(TAB_SIZE-1, meanstd(recall)) << " "
         << centered(TAB_SIZE-1, meanstd(precision)) << " "
         << centered(TAB_SIZE-1, meanstd(Fbeta)) << " "
         << centered(TAB_SIZE-1, meanstd(NPV)) << " "
         << centered(TAB_SIZE-1, meanstd(TNR)) << " "
         << endl;
  }

  /* print EAD */
  if (!c.exist("no-ead")) {
    if (!c.exist("no-confusion") || !c.exist("no-score"))
      cout << endl;

    //calculate_ead();

    uint64_t ev_total = ead.deletions + ead.ev_fragmented + ead.ev_fragmerged +
                        ead.ev_merged + ead.correct,
             re_total = ead.correct + ead.re_merged + ead.re_fragmerged +
                        ead.re_fragmented + ead.insertions;
    float total = ev_total + re_total - ead.correct;
    float LINE_SIZE = 80 - 3*8,
             d = ead.deletions / total,
             ef = ead.ev_fragmented / total, efm = ead.ev_fragmerged / total,
             em = ead.ev_merged / total, c = ead.correct / total,
             rm = ead.re_merged / total, rfm = ead.re_fragmerged / total,
             rf = ead.re_fragmented / total, i = ead.insertions / total;

    if (total > 0) {
      cout << string(d*LINE_SIZE   +4, u'-')   << " " <<
              string(ef*LINE_SIZE  +4, '-')  << " " <<
              string(efm*LINE_SIZE +4, '-') << " " <<
              string(em*LINE_SIZE  +4, '-')  << " " <<
              string(c*LINE_SIZE   +4, '-')   << endl;

      cout << centered(d*LINE_SIZE   +4,"D",4) << " " <<
              centered(ef*LINE_SIZE  +4,"F",4) << " " <<
              centered(efm*LINE_SIZE +4,"FM",4) << " " <<
              centered(em*LINE_SIZE  +4,"M",4) << " " <<
              centered(c*LINE_SIZE   +4,"C",4) << " " <<
              centered(rm*LINE_SIZE  +4,"M",4) << " " <<
              centered(rfm*LINE_SIZE +4,"FM",4) << " " <<
              centered(rf*LINE_SIZE  +4,"F",4) << " " <<
              centered(i*LINE_SIZE   +4,"I",4) << endl;

      cout << centered(d*LINE_SIZE   +4,100*d,4) << " " <<
              centered(ef*LINE_SIZE  +4,100*ef,4) << " " <<
              centered(efm*LINE_SIZE +4,100*efm,4) << " " <<
              centered(em*LINE_SIZE  +4,100*em,4) << " " <<
              centered(c*LINE_SIZE   +4,100*c,4) << " " <<
              centered(rm*LINE_SIZE  +4,100*rm,4) << " " <<
              centered(rfm*LINE_SIZE +4,100*rfm,4) << " " <<
              centered(rf*LINE_SIZE  +4,100*rf,4) << " " <<
              centered(i*LINE_SIZE   +4,100*i,4) << endl;

      cout << centered(d*LINE_SIZE   +4,ead.deletions,4) << " " <<
              centered(ef*LINE_SIZE  +4,ead.ev_fragmented,4) << " " <<
              centered(efm*LINE_SIZE +4,ead.ev_fragmerged,4) << " " <<
              centered(em*LINE_SIZE  +4,ead.ev_merged,4) << " " <<
              centered(c*LINE_SIZE   +4,ead.correct,4) << " " <<
              centered(rm*LINE_SIZE  +4,ead.re_merged,4) << " " <<
              centered(rfm*LINE_SIZE +4,ead.re_fragmerged,4) << " " <<
              centered(rf*LINE_SIZE  +4,ead.re_fragmented,4) << " " <<
              centered(i*LINE_SIZE   +4,ead.insertions,4) << endl;

      cout << string(d*LINE_SIZE   +4, ' ')   << " " <<
              string(ef*LINE_SIZE  +4, ' ')  << " " <<
              string(efm*LINE_SIZE +4, ' ') << " " <<
              string(em*LINE_SIZE  +4, ' ')  << " " <<
              string(c*LINE_SIZE   +4, '-')   << " " <<
              string(rm*LINE_SIZE  +4, '-')  << " " <<
              string(rfm*LINE_SIZE +4, '-') << " " <<
              string(rf*LINE_SIZE  +4, '-')  << " " <<
              string(i*LINE_SIZE   +4, '-')   << endl;
    } else {
      cout << "total is zero" << endl;
    }

    //cout << "deletions: " << ead.deletions << endl;
    //cout << "ev_fragmented: " << ead.ev_fragmented << endl;
    //cout << "ev_fragmerged: " << ead.ev_fragmerged << endl;
    //cout << "ev_merged: " << ead.ev_merged << endl;
    //cout << "correct: " << ead.correct << endl;
    //cout << "re_merged: " << ead.re_merged << endl;
    //cout << "re_fragmerged: " << ead.re_fragmerged << endl;
    //cout << "re_fragmented: " << ead.re_fragmented << endl;
    //cout << "insertions: " << ead.insertions << endl;
  }

  return cout.str();
}

string centered(int tab_size, string val, int DEFAULT) {
  stringstream ss;
  if (tab_size < DEFAULT) tab_size = DEFAULT;
  //if (val.size() > tab_size-DEFAULT+1) val.resize(tab_size-DEFAULT+1);
  if (val.size() > tab_size) val.resize(tab_size);

  int pre = (tab_size - val.size()) / 2,
     post =  tab_size - val.size() - pre;

  ss << string(pre,' ') << val << string(post, ' ');
  return ss.str();
}

string centered(int tab_size, double value, int DEFAULT) {
  return centered(tab_size, to_string(value), DEFAULT);
}

string centered(int tab_size, uint64_t value, int DEFAULT) {
  return centered(tab_size, to_string(value), DEFAULT);
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
  T result = 0;
  for (int i=0; i<m.getNumRows(); i++)
    for (int j=0; j<m.getNumCols(); j++)
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

template< class T>
std::vector<T> operator+(const std::vector<T> &a, const std::vector<T> &b)
{
  std::vector<T> res(a.size());
  for(size_t i=0; i<a.size(); ++i)
    res[i]=a[i]+b[i];
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
    else
      cleaned.push_back(0.);

  if (cleaned.size() == 0)
    return "";

  double mean = sum(cleaned)/cleaned.size();

  if (cleaned.size() == 1)
    return to_string(mean);

  stringstream ss;
  ss << mean << "/" << sqrt( sum(pow(abs(cleaned-mean),2))/cleaned.size() );
  return ss.str();
}

string mean(vector< double > list) {
  vector< double > cleaned;

  for ( auto val : list )
    if (!std::isnan(val))
      cleaned.push_back(val);
    else
      cleaned.push_back(0.);

  if (cleaned.size() == 0)
    return "";

  double mean = sum(cleaned)/cleaned.size();
  return to_string(mean);
}
