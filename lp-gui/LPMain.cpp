#include "LPMain.h"
#include "ui_LPMain.h"

LPMain::LPMain(QWidget *parent) : QMainWindow(parent), ui(new Ui::LPMain){
  ui->setupUi(this); //load the Qt-designer UI file
  //Create the basic/advanced view options
  viewBasic = new QRadioButton(tr("Basic"), ui->menuView);
	QWidgetAction *WABasic = new QWidgetAction(this); WABasic->setDefaultWidget(viewBasic);
  viewAdvanced = new QRadioButton(tr("Advanced"), ui->menuView);
	QWidgetAction *WAAdv = new QWidgetAction(this); WAAdv->setDefaultWidget(viewAdvanced);
	
  ui->menuView->addAction(WABasic);
  ui->menuView->addAction(WAAdv);
  connect(viewBasic, SIGNAL(toggled(bool)), this, SLOT(viewChanged()) );
  //Now set the default view type
  viewBasic->setChecked(true); //will automatically call the "viewChanged" function
	
  //Connect the UI to all the functions
  connect(ui->combo_pools, SIGNAL(currentIndexChanged(int)), this, SLOT(updateTabs()) );
  connect(ui->combo_datasets, SIGNAL(currentIndexChanged(int)), this, SLOT(updateDataset()) );
  connect(ui->slider_snapshots, SIGNAL(valueChanged(int)), this, SLOT(updateSnapshot()) );
  //Update the interface
  updatePoolList();
  //Make sure the status tab is shown initially
  ui->tabWidget->setCurrentWidget(ui->tab_status);
}

LPMain::~LPMain(){
	
}

// ==============
//      PUBLIC SLOTS
// ==============
void LPMain::slotSingleInstance(){
  this->raise();
  this->show();
}

// ==============
//          PRIVATE
// ==============

// ==============
//     PRIVATE SLOTS
// ==============
void LPMain::updatePoolList(){
  //Get the currently selected pool (if there is one)
  QString cPool;
  if(ui->combo_pools->currentIndex() != -1){ cPool = ui->combo_pools->currentText(); }
  //Get the list of managed pools
  QStringList pools = LPBackend::listDatasets();
  //Now put that list into the UI
  ui->combo_pools->clear();
  if(!pools.isEmpty()){ ui->combo_pools->addItems(pools); }
  //Now set the currently selected pools
  if(pools.length() > 0){
    int index = pools.indexOf(cPool);
    if(index < 0){ ui->combo_pools->setCurrentIndex(0); }
    else{ ui->combo_pools->setCurrentIndex(index); }
    poolSelected=true;
  }else{
    //No managed pools
    ui->combo_pools->addItem("No Managed Pools!");
    ui->combo_pools->setCurrentIndex(0);
    poolSelected=false;
  }
  //Now update the interface appropriately
  ui->combo_pools->setEnabled(poolSelected);
  updateTabs();
}

void LPMain::viewChanged(){
  ui->menubar->clear();
  if(viewBasic->isChecked()){
    ui->menubar->addMenu(ui->menuFile);
    ui->menubar->addMenu(ui->menuView);
  }else{
    ui->menubar->addMenu(ui->menuFile);
    ui->menubar->addMenu(ui->menuView);
    ui->menubar->addMenu(ui->menuDisks);
    ui->menubar->addMenu(ui->menuSnapshots);
  }
}

void LPMain::updateTabs(){
  viewChanged();
  ui->tabWidget->setEnabled(poolSelected);
  ui->menuView->setEnabled(poolSelected);	
  ui->tool_configure->setVisible(poolSelected);
  ui->tool_configBackups->setVisible(poolSelected);
  ui->actionUnmanage_Pool->setEnabled(poolSelected);
  ui->action_SaveKeyToUSB->setEnabled(poolSelected);
  if(poolSelected){
    POOLDATA = LPGUtils::loadPoolData(ui->combo_pools->currentText());
    //Now list the status information
    ui->label_status->setText(POOLDATA.poolStatus);
    ui->label_numdisks->setText(POOLDATA.numberOfDisks);
    ui->label_latestsnapshot->setText(POOLDATA.latestSnapshot);
    if(POOLDATA.replicationStatus.isEmpty()){ ui->label_replication->setVisible(false); }
    else{
      ui->label_replication->setText(POOLDATA.replicationStatus);
      ui->label_replication->setVisible(true);
    }
    if(POOLDATA.mirrorStatus.isEmpty()){ ui->label_mirror->setVisible(false); }
    else{
      ui->label_mirror->setText(POOLDATA.mirrorStatus);
      ui->label_mirror->setVisible(true);
    }	    
    if(POOLDATA.errorStatus.isEmpty()){ ui->label_errors->setVisible(false); }
    else{
      ui->label_errors->setText(POOLDATA.errorStatus);
      ui->label_errors->setVisible(true);
    }	    
    //Now list the data restore options
    QString cds = ui->combo_datasets->currentText();
    ui->combo_datasets->clear();
    QStringList dslist = POOLDATA.subsets();
    qDebug() << "Datasets:" << dslist;
    ui->combo_datasets->addItems(dslist);
    int dsin = dslist.indexOf(cds);
    if(dsin >= 0){ ui->combo_datasets->setCurrentIndex(dsin); }
    else if( !dslist.isEmpty() ){ ui->combo_datasets->setCurrentIndex(0); }
    else{ ui->combo_datasets->addItem(tr("No datasets available")); }
    //Automatically calls the "updateDataset()" function
  }else{
    //No Pool selected
    ui->menuDisks->setEnabled(false); //make sure this is always invisible if nothing selected
    ui->menuSnapshots->setEnabled(false); //make sure this is always invisible if nothing selected
    ui->label_numdisks->clear();
    ui->label_latestsnapshot->clear();
    ui->label_status->clear();
	  ui->label_errors->setVisible(false);
	  ui->label_mirror->setVisible(false);
	  ui->label_replication->setVisible(false);
  }

}

void LPMain::updateDataset(){
  //Update the snapshots for the currently selected dataset
  QString cds = ui->combo_datasets->currentText();
  if(POOLDATA.subsets().indexOf(cds) >= 0){
    QStringList snaps = POOLDATA.snapshots(cds);
      qDebug() << "dataset:" << cds << "snapshots:" << snaps;
      ui->slider_snapshots->setEnabled(true);
      ui->slider_snapshots->setMinimum(0);
      int max = snaps.length() -1;
      if(max < 0){ max = 0; ui->slider_snapshots->setEnabled(false); }
      ui->slider_snapshots->setMaximum(max);
      ui->slider_snapshots->setValue(max); //most recent snapshot
      updateSnapshot();
  }else{
    ui->slider_snapshots->setEnabled(false);
    ui->label_snapshot->clear();
    ui->tool_nextsnap->setEnabled(false);
    ui->tool_prevsnap->setEnabled(false);
  }
	
}

void LPMain::updateSnapshot(){
  int sval = ui->slider_snapshots->value();
  QStringList snaps = POOLDATA.snapshots(ui->combo_datasets->currentText());
  qDebug() << "Snapshots:" << snaps;
  //Update the previous/next buttons
  if(sval == ui->slider_snapshots->minimum() ){ ui->tool_prevsnap->setEnabled(false); }
  else{ ui->tool_prevsnap->setEnabled(true); }
  if(sval == ui->slider_snapshots->maximum() ){ ui->tool_nextsnap->setEnabled(false); }
  else{ ui->tool_nextsnap->setEnabled(true); }
  //Now update the snapshot viewer
  if(snaps.isEmpty()){ ui->label_snapshot->clear(); ui->slider_snapshots->setEnabled(false); }
  else{
    QString snap = snaps.at(sval);
    ui->label_snapshot->setText(snap);
    //Now update the snapshot view
    qDebug() << "Snapshot viewer not implemented yet";
  }
}