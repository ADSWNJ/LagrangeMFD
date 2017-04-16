//
// Glideslope Button Control ... leveraging Enjo's MFDButtons helper library
//
// Purpose ... sets up the menus and supports the context page switching (main, config, deorbit menus)
//
// (c) Andrew Stokes (ADSWNJ) 2012-2017
//
// All rights reserved
//


#ifndef _GSmfdButtonClass
#define _GSmfdButtonClass

class GlideslopeMFD;

class GSmfdButtons : public MFDButtonPage<GlideslopeMFD>
{
  public:
    GSmfdButtons();
  protected:
    bool SearchForKeysInOtherPages() const;
  private:
};
#endif

