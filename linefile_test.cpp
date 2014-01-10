int main(int argc,char **argv)
{
    int t,IDt,Np,Ns;
    double p[3],d[3];
    string fn("FourLayer.bad");
    LineFile lf(fn,'%',cout);

    lf >> Ns >> LineFile::end_line;
    lf >> t >> IDt >> p[0] >> p[1] >> p[2] >> d[0] >> d[1] >> d[2] >> Np >> LineFile::end_line;

    cout << "Source at (" << p[0] << ',' << p[1] << ',' << p[2] << ") directed (" << d[0] << ',' << d[1] << ',' << d[2] << ") IDt=" << IDt << " Np=" << Np << endl;

    lf >> t >> IDt >> p[0] >> p[1] >> p[2] >> d[0] >> d[1] >> d[2] >> Np >> LineFile::skip_nls >> LineFile::end_file;

    cout << "Source at (" << p[0] << ',' << p[1] << ',' << p[2] << ") directed (" << d[0] << ',' << d[1] << ',' << d[2] << ") IDt=" << IDt << " Np=" << Np << endl;

    return 0;
}
