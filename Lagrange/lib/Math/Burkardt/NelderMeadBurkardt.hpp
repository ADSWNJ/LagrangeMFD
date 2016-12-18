
namespace EnjoLib{
class OptiMultiSubject;
}

namespace NelderMeadBurkardt
{
    void nelmin ( EnjoLib::OptiMultiSubject & subj, int n, double start[], double xmin[],
      double *ynewlo, double reqmin, const double step[], int konvge, int kcount,
      int *icount, int *numres, int *ifault );
    void timestamp ( );
}
