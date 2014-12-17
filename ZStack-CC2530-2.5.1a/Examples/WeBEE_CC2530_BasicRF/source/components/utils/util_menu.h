/*********************************************************************
    
    Filename:       util_menu.h

    Description:    UART based one-key menu.

*********************************************************************/

#ifndef UTIL_MENU_H
#define UTIL_MENU_H

#ifdef __cplusplus
extern "C"
{
#endif

/*********************************************************************
 * INCLUDES
 */


#include "hal_types.h"


/*********************************************************************
 * CONSTANTS
 */

#define MENU_ID_APP     0

#define MENU_ID_ROOT    1
#define MENU_ID_NWK     2
#define MENU_ID_HAL     3
#define N_MAX_MENUES    4


/*********************************************************************
 * TYPEDEFS
 */

typedef struct {
    uint8 cmd;				// Single key user command
    const char FAR *szDecsr;// Textual description
    void (*pfCmd)(void);    // Function; 0 arguments
} MenuItem;


typedef struct _menuItem {
    struct _menuItem *pParent;
    const char FAR *szPrompt;
    MenuItem FAR *pFirst;
} Menu;

/*********************************************************************
 * PUBLIC FUNCTIONS
 */

void menuInit(void);
void menuInstall(uint8 iMenuID, MenuItem FAR *pMenuHandle, const char FAR *szPr);
uint8 menuCmdExec(uint8 *pBuf, uint8 nBytes);
void menuPrint(void);
void menuPrintPrompt(void);
void menuPrintItem(MenuItem FAR *p);
uint8 menuIsInstalled(uint8 iMenuID);
void menuSelect(uint8 iMenuID);
MenuItem FAR *menuFindItem(Menu *pMenu, uint8 cmd);

#ifdef __cplusplus
}
#endif

#endif

