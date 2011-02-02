/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/

void MarteMainForm::init()
{
 MarteTabWidget->removePage(MarteTabWidget->currentPage());
    addNewControl();
}

void MarteMainForm::destroy()
{
}

void MarteMainForm::addNewControl()
{
    MarteTabWidget->addTab((new MarteControl()), "Marte Control");
    connect(MarteTabWidget->page(MarteTabWidget->count() - 1), SIGNAL(AddTab()), this, SLOT(addNewControl()));
    connect(MarteTabWidget->page(MarteTabWidget->count() - 1), SIGNAL(RemoveTab()), this, SLOT(removeCurrentControl()));   
}

void MarteMainForm::removeCurrentControl()
{
    if(MarteTabWidget->count() > 1)
    {
  MarteControl* delPtr = (MarteControl*) MarteTabWidget->currentPage();
  MarteTabWidget->removePage(MarteTabWidget->currentPage());
  delete delPtr;
    } 
}
