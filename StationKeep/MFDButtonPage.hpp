// ==============================================================
//          ORBITER AUX LIBRARY: Multiple Buttons Pages
//             http://sf.net/projects/enjomitchsorbit
//                  Part of the ORBITER SDK
//
// Copyright (C) 2012      Szymon "Enjo" Ender
//
//                         All rights reserved
//
// Multiple Buttons Pages is free software: you can redistribute it
// and/or modify it under the terms of the GNU Lesser General Public
// License as published by the Free Software Foundation, either version
// 3 of the License, or (at your option) any later version.
//
// Multiple Buttons Pages is distributed in the hope that it will
// be useful, but WITHOUT ANY WARRANTY; without even the implied
// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with Multiple Vessels Support. If not, see
// <http://www.gnu.org/licenses/>.
// ==============================================================

#ifndef MFDBUTTONPAGE_H
#define MFDBUTTONPAGE_H

#include <OrbiterSDK.h>
#include <map>
#include <vector>

/// Manages MFD button pages.
/**
    Allows to register many switchable MFD button pages of the MFDClass and associates handlers
    for the buttons. The handlers must reside in the MFDClass, and they must not have any arguments.
    \tparam <MFDClass>  Your MFD class that should contain handlers
*/
template <class MFDClass>
class MFDButtonPage
{
    public:
        /// Ctor
        MFDButtonPage();
        /// Dtor
        virtual ~MFDButtonPage();

        /// Switches between button pages
        /**
            A button for this method needs to be registered through RegisterFunction(),
            which should call MFDClass' method, which then calls this method. Example:
            \code{.cpp}
            MFDButtonPageLaunchMFD::MFDButtonPageLaunchMFD()
            {
                // ... add menu first, then functions
                RegisterFunction("PG",  OAPI_KEY_P, &LaunchMFD::SwitchButtonsPage);
                // ...
            }
            \endcode
            \code{.cpp}
            void LaunchMFD::SwitchButtonsPage()
            {
                m_buttonPages.SwitchPage(this);
            }
            \endcode

            \param mfdInstance - Instance of your MFD class
        */
        void SwitchPage( MFDClass * mfdInstance, int page = -1 ) const;

        /// Reacts on mouse clicks on buttons
        /**
            Call from MFD::ConsumeButton(). Example:
            \code{.cpp}
            bool LaunchMFD::ConsumeButton (int bt, int event)
            {
                return m_buttonPages.ConsumeButton(this, bt, event);
            }
            \endcode
            \param mfdInstance - Instance of your MFD class (this)
            \param button - button number
            \param event - Windows event
        */
        bool ConsumeButton( MFDClass * mfdInstance, int button, int event ) const;

        /// Reacts on keyboard presses
        /**
            Searches for the provided key in a map for the selected page and fires associated function.
            If SearchForKeysInOtherPages() returns true, then other pages are searched for as well.
            Call from MFD::ConsumeKeyBuffered(). Example:
            \code{.cpp}
            bool LaunchMFD::ConsumeKeyBuffered(DWORD key)
            {
                return m_buttonPages.ConsumeKeyBuffered(this, key);
            }
            \endcode
            \param mfdInstance - Instance of your MFD class (this)
            \param key - key designation, like OAPI_KEY_T
        */
        bool ConsumeKeyBuffered( MFDClass * mfdInstance, DWORD key ) const;

        /// Returns button menu of the current page
        /**
            Call from MFD::ButtonMenu(). Example:
            \code{.cpp}
            int LaunchMFD::ButtonMenu (const MFDBUTTONMENU **menu) const
            {
                return m_buttonPages.ButtonMenu( menu );
            }
            \endcode
            \param menu - this will be initialised by internal structure
        */
        int ButtonMenu (const MFDBUTTONMENU **menu) const;

        /// Returns button labels of the current page
        /**
            Call from MFD::ButtonLabel(). Example:
            \code{.cpp}
            char * LaunchMFD::ButtonLabel (int bt)
            {
                return m_buttonPages.ButtonLabel(bt);
            }
            \endcode
            \param button - button number
        */
        char * ButtonLabel (int button) const;

    protected:
        /// Should other pages be searched for when pressing a key?
        /**
            Normally the system reacts only keys on the current page.
            If this method returns true, then other pages are searched for as well.
        */
        virtual bool SearchForKeysInOtherPages() const = 0;

        /// Function pointers of the MFDClass
        typedef void (MFDClass::* MFDFunctionPtr)(void);

        /// Registeres handler in MFD scope for buttons.
        /**
            Before this method has any effect, a button page needs to be registered first with
            RegisterPage(). The keys may be repeated in different pages.
            \code{.cpp}
            MFDButtonPageLaunchMFD::MFDButtonPageLaunchMFD()
            {
                // ... add menu first, then functions
                RegisterFunction("TGT", OAPI_KEY_T, &LaunchMFD::OpenDialogTarget);
                RegisterFunction("ALT", OAPI_KEY_A, &LaunchMFD::OpenDialogAltitude);
                RegisterFunction("PG", OAPI_KEY_P, &LaunchMFD::SwitchButtonsPage);
            }
            \endcode
            \param label - 3 char long label of button. Example: "TGT"
            \param key - associated key on keyboard, one of OAPI_KEY_*. Example: OAPI_KEY_T
            \param f - MFD function pointer (handler). Example: & MyMFD::OpenDialogTarget. The handler can"t take any arguments.
        */
        void RegisterFunction( const std::string & label, DWORD key, MFDFunctionPtr f );

        /// Registeres button page, and buttons menu
        /**
            Must be called before RegisterFunction() is called.
            The menu must be \b STATICALLY declared. For example:
            \code{.cpp}
            static const MFDBUTTONMENU mnu1[] =
            {
                {"Select target", 0, 'T'},
                {"Enter altitude", "km", 'A'},
                {"Switch button page", 0, 'P'},
            };
            RegisterPage(mnu1, sizeof(mnu1) / sizeof(MFDBUTTONMENU));
            \endcode
            \param menu - \b STATICALLY declared button menu
            \param size - menu size. Use sizeof(menu) / sizeof(MFDBUTTONMENU)
        */
        void RegisterPage( const MFDBUTTONMENU * menu, int size );

    private:

        /// Represents button page
        struct Page
        {
            const MFDBUTTONMENU * m_menu;
            int m_menuSize;
            std::vector<std::string> m_labels;
            std::vector<MFDFunctionPtr> m_buttons;
            std::map<DWORD, MFDFunctionPtr> m_keys;
        };

        std::vector<Page> m_pages; ///< Current page index
        mutable size_t m_i; ///< Current page index
};

// Public:
template <class MFDClass>
MFDButtonPage<MFDClass>::MFDButtonPage()
: m_i(0)
{
}

template <class MFDClass>
MFDButtonPage<MFDClass>::~MFDButtonPage() {}

template <class MFDClass>
void MFDButtonPage<MFDClass>::SwitchPage( MFDClass * mfdInstance, int page = -1 )  const
{
    if (page == -1) page = (m_i++); // e.g. use SwitchPage(mfd) to increment page, and SwitchPage(mfd,2) to select specific page 2. 
    m_i = (page) % m_pages.size(); // increment index, but not beyond the size
    mfdInstance->InvalidateButtons(); // redraw buttons
}

template <class MFDClass>
bool MFDButtonPage<MFDClass>::ConsumeButton( MFDClass * mfdInstance, int button, int event ) const
{
    if (event & PANEL_MOUSE_LBDOWN) // Left mouse button just clicked
    {
        if ( button < (int)m_pages.at(m_i).m_buttons.size())
        {
            (mfdInstance->*(m_pages.at(m_i).m_buttons[button]))(); // Call the function
            return true;
        }
    }
    return false;
}

template <class MFDClass>
bool MFDButtonPage<MFDClass>::ConsumeKeyBuffered( MFDClass * mfdInstance, DWORD key ) const
{
    // First search for the key on this page
    std::map<DWORD, MFDFunctionPtr>::const_iterator it = m_pages.at(m_i).m_keys.find(key);
    if (it != m_pages.at(m_i).m_keys.end() )
    {
        (mfdInstance->*(it->second))(); // Call the function
        return true;
    }

    if ( SearchForKeysInOtherPages() )
    {
        // Then, if required, search in other pages
        for ( size_t j = 0; j < m_pages.size(); ++j )
        {
            if ( m_i == j )
                continue; // The current page was already queried
            std::map<DWORD, MFDFunctionPtr>::const_iterator it = m_pages.at(j).m_keys.find(key);
            if (it != m_pages.at(j).m_keys.end() )
            {
                (mfdInstance->*(it->second))();  // Call the function
                return true;
            }
        }
    }

    return false;
}

template <class MFDClass>
int MFDButtonPage<MFDClass>::ButtonMenu (const MFDBUTTONMENU **menu) const
{
    if ( menu ) *menu = m_pages.at(m_i).m_menu; // pass the static menu to the parameric pointer
    return m_pages.at(m_i).m_menuSize;
}

template <class MFDClass>
char * MFDButtonPage<MFDClass>::ButtonLabel (int bt) const
{
    return (bt < (int)m_pages.at(m_i).m_labels.size() ?
             (char*)m_pages.at(m_i).m_labels[bt].c_str() : NULL);
}

// Private:
template <class MFDClass>
void MFDButtonPage<MFDClass>::RegisterFunction( const std::string & label, DWORD key, MFDFunctionPtr f )
{
    if ( m_pages.empty() )
    {
        sprintf_s(oapiDebugString(), 512, "MFDButtonPage::RegisterFunction(): No pages registered yet!");
        return;
    }
    Page & p = m_pages.back();

    p.m_labels.push_back(label);
    p.m_buttons.push_back(f);
    p.m_keys[key] = f;
}

template <class MFDClass>
void MFDButtonPage<MFDClass>::RegisterPage( const MFDBUTTONMENU * menu, int size )
{
    Page p;
    p.m_menu = menu;
    p.m_menuSize = size;
    m_pages.push_back(p);
}

#endif // MFDBUTTONPAGE_H