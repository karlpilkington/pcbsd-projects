#include "LPMain.h"
#include "ui_LPMain.h"

LPMain::LPMain(QWidget *parent) : QMainWindow(parent), ui(new Ui::LPMain){
  ui->setupUi(this); //load the Qt-designer UI file
  //Create the basic/advanced view options
  viewBasic = new QRadioButton(tr("Basic"), ui->menuView);
	QWidgetAction *WABasic = new QWidgetAction(this); WABasic->setDefaultWidget(viewBasic);
	ui->menuView->addAction(WABasic);
  viewAdvanced = new QRadioButton(tr("Advanced"), ui->menuView);
	QWidgetAction *WAAdv = new QWidgetAction(this); WAAdv->setDefaultWidget(viewAdvanced);
	ui->menuView->addAction(WAAdv);
  connect(viewBasic, SIGNAL(toggled(bool)), this, SLOT(viewChanged()) );
  //Now set the default view type
  viewBasic->setChecked(true); //will automatically call the "viewChanged" function
  //Create the filesystem model and tie it to the treewidget
  fsModel = new QFileSystemModel(this);
	fsModel->setReadOnly(true);
	//fsModel->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot );
	ui->treeView->setModel(fsModel);
  //Create the menu's for the special menu actions
	
	
  //Connect the UI to all the functions
  connect(ui->combo_pools, SIGNAL(currentIndexChanged(int)), this, SLOT(updateTabs()) );
  connect(ui->combo_datasets, SIGNAL(currentIndexChanged(int)), this, SLOT(updateDataset()) );
  connect(ui->slider_snapshots, SIGNAL(valueChanged(int)), this, SLOT(updateSnapshot()) );
  connect(ui->push_prevsnap, SIGNAL(clicked()), this, SLOT(prevSnapshot()) );
  connect(ui->push_nextsnap, SIGNAL(clicked()), this, SLOT(nextSnapshot()) );
  connect(ui->check_hidden, SIGNAL(stateChanged(int)), this, SLOT(setFileVisibility()) );
  connect(ui->push_restore, SIGNAL(clicked()), this, SLOT(restoreFiles()) );
  connect(ui->push_configure, SIGNAL(clicked()), this, SLOT(openConfigGUI()) );
  connect(ui->push_configBackups, SIGNAL(clicked()), this, SLOT(openBackupGUI()) );
  //Connect the Menu buttons
  connect(ui->menuManage_Pool, SIGNAL(triggered(QAction*)), this, SLOT(menuAddPool(QAction*)) );
  connect(ui->menuUnmanage_Pool, SIGNAL(triggered(QAction*)), this, SLOT(menuRemovePool(QAction*)) );
  connect(ui->action_SaveKeyToUSB, SIGNAL(triggered()), this, SLOT(menuSaveSSHKey()) );
  connect(ui->actionClose_Window, SIGNAL(triggered()), this, SLOT(menuCloseWindow()) );
  connect(ui->actionCompress_Home_Dir, SIGNAL(triggered()), this, SLOT(menuCompressHomeDir()) );
  connect(ui->actionExtract_Home_Dir, SIGNAL(triggered()), this, SLOT(menuExtractHomeDir()) );
  connect(ui->actionAdd_Disk, SIGNAL(triggered()), this, SLOT(menuAddDisk()) );
  connect(ui->actionRemove_Disk, SIGNAL(triggered()), this, SLOT(menuRemoveDisk()) );
  connect(ui->actionSet_Disk_Offline, SIGNAL(triggered()), this, SLOT(menuOfflineDisk()) );
  connect(ui->action_startScrub, SIGNAL(triggered()), this, SLOT(menuStartScrub()) );
  connect(ui->action_newSnapshot, SIGNAL(triggered()), this, SLOT(menuNewSnapshot()) );
  connect(ui->action_rmSnapshot, SIGNAL(triggered()), this, SLOT(menuRemoveSnapshot()) );
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
void LPMain::showErrorDialog(QString title, QString message, QString errors){
  QMessageBox MB(QMessageBox::Warning, title, message, QMessageBox::Ok, this);
    MB.setDetailedText(errors);
    MB.exec();
}

// ==============
//     PRIVATE SLOTS
// ==============
void LPMain::updatePoolList(){
  //Get the currently selected pool (if there is one)
  qDebug() << "Update Pool List";
  QString cPool;
  if(ui->combo_pools->currentIndex() != -1){ cPool = ui->combo_pools->currentText(); }
  //Get the list of managed pools
  QStringList pools = LPBackend::listDatasets();
  QStringList poolsAvail = LPBackend::listPossibleDatasets();
  //Now put the lists into the UI
  ui->combo_pools->clear();
  if(!pools.isEmpty()){ ui->combo_pools->addItems(pools); }
  //Now set the currently selected pools
  if(pools.length() > 0){
    poolSelected=true;	  
    int index = pools.indexOf(cPool);
    if(index < 0){ ui->combo_pools->setCurrentIndex(0); }
    else{ ui->combo_pools->setCurrentIndex(index); }
  }else{
    //No managed pools
    poolSelected=false;
    ui->combo_pools->addItem("No Managed Pools!");
    ui->combo_pools->setCurrentIndex(0);
  }
  //Now update the add/remove pool menu's
  ui->menuManage_Pool->clear();
  for( int i=0; i<poolsAvail.length(); i++){
    if(pools.contains(poolsAvail[i])){ continue; } //already managed
    ui->menuManage_Pool->addAction(poolsAvail[i]);
  }
  ui->menuManage_Pool->setEnabled( !ui->menuManage_Pool->isEmpty() );
  ui->menuUnmanage_Pool->clear();
  for( int i=0; i<pools.length(); i++){
    ui->menuUnmanage_Pool->addAction(pools[i]);
  }
  ui->menuUnmanage_Pool->setEnabled( !ui->menuUnmanage_Pool->isEmpty() );
  
  //Now update the interface appropriately
  ui->combo_pools->setEnabled(poolSelected);
  updateTabs();
}

void LPMain::viewChanged(){
  ui->menubar->clear();
  if(viewBasic->isChecked()){
    ui->menubar->addMenu(ui->menuFile);
    ui->menubar->addMenu(ui->menuView);
    ui->menubar->addMenu(ui->menuClassic_Backups);
  }else{
    ui->menubar->addMenu(ui->menuFile);
    ui->menubar->addMenu(ui->menuView);
    ui->menubar->addMenu(ui->menuClassic_Backups);
    ui->menubar->addMenu(ui->menuDisks);
    ui->menubar->addMenu(ui->menuSnapshots);
  }
}

void LPMain::updateTabs(){
  qDebug() << "Update Tabs" << poolSelected;
  viewChanged();
  ui->tabWidget->setEnabled(poolSelected);
  ui->menuView->setEnabled(poolSelected);	
  ui->menuDisks->setEnabled(poolSelected); 
  ui->menuSnapshots->setEnabled(poolSelected);
  ui->push_configure->setVisible(poolSelected);
  ui->push_configBackups->setVisible(poolSelected);
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
    ui->combo_datasets->addItems(dslist);
    int dsin = dslist.indexOf(cds);
    if(dsin >= 0){ ui->combo_datasets->setCurrentIndex(dsin); }
    else if( !dslist.isEmpty() ){ ui->combo_datasets->setCurrentIndex(0); }
    else{ ui->combo_datasets->addItem(tr("No datasets available")); }
    //Automatically calls the "updateDataset()" function
  }else{
    //No Pool selected
    ui->menuDisks->setEnabled(false); //make sure this is always disabled if nothing selected
    ui->menuSnapshots->setEnabled(false); //make sure this is always disabled if nothing selected
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
      qDebug() << "Update Dataset";
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
    ui->push_nextsnap->setEnabled(false);
    ui->push_prevsnap->setEnabled(false);
  }
	
}

void LPMain::updateSnapshot(){
  int sval = ui->slider_snapshots->value();
  QStringList snaps = POOLDATA.snapshots(ui->combo_datasets->currentText());
  qDebug() << "Update Snapshot";
  //Update the previous/next buttons
  if(sval == ui->slider_snapshots->minimum() ){ ui->push_prevsnap->setEnabled(false); }
  else{ ui->push_prevsnap->setEnabled(true); }
  if(sval == ui->slider_snapshots->maximum() ){ ui->push_nextsnap->setEnabled(false); }
  else{ ui->push_nextsnap->setEnabled(true); }
  //Now update the snapshot viewer
  if(snaps.isEmpty()){ ui->label_snapshot->clear(); ui->slider_snapshots->setEnabled(false); }
  else{
    QString snap = snaps.at(sval);
    QString path = ui->combo_datasets->currentText() + "/.zfs/snapshot/"+snap;
    //qDebug() << "Snapshot path:" << path;
    ui->label_snapshot->setText(snap);
    //Now update the snapshot view
    ui->treeView->setRootIndex( fsModel->setRootPath(path) );
    
  }
}

void LPMain::nextSnapshot(){
  ui->slider_snapshots->setValue( ui->slider_snapshots->value()+1 );
}

void LPMain::prevSnapshot(){
  ui->slider_snapshots->setValue( ui->slider_snapshots->value()-1 );
}

void LPMain::setFileVisibility(){
  if(ui->check_hidden->isChecked()){
    fsModel->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot | QDir::Hidden );
  }else{
    fsModel->setFilter(QDir::AllDirs | QDir::Files | QDir::NoDotAndDotDot );
  }	  
}

void LPMain::restoreFiles(){
  QString filePath = fsModel->filePath( ui->treeView->currentIndex() );
  qDebug() << " Restore file(s):" << filePath;
  QString destDir = filePath.remove("/.zfs/snapshot/"+ui->label_snapshot->text());
	destDir.chop( filePath.section("/",-1).size()+1 ); //get rid of the filename at the end
	while(!QFile::exists(destDir)){ destDir.chop( destDir.section("/",-1).size() +1); }
  QString newFilePath = destDir+"/"+LPGUtils::generateReversionFileName(filePath, destDir);
  //qDebug() << "Destination:" << newFilePath;
  //Perform the reversion(s)
  QStringList errors;
  if(QFileInfo(filePath).isDir()){
    //Is a directory
    errors = LPGUtils::revertDir(filePath, newFilePath);
    if(!errors.isEmpty()){
      qDebug() << "Failed Reversions:" << errors;
      errors.prepend(tr("File destination(s) that could not be restored:")+"\n");
      showErrorDialog(tr("Reversion Error"), tr("Some files could not be restored from the snapshot."), errors.join("\n") );
    }else{
      qDebug() << "Reversion successful";	    
      QMessageBox::information(this,tr("Restore Successful"),QString(tr("The following directory was succesfully restored: %1")).arg(newFilePath) );
    }
  }else{
    //Just a single file
    bool ok = LPGUtils::revertFile(filePath, newFilePath);
    if( !ok ){
      qDebug() << "Failed Reversion:" << newFilePath;
      errors << QString(tr("Snapshot file: %1")).arg(filePath);
      errors << QString(tr("Destination: %1")).arg(newFilePath);
      errors << tr("Please check that the destination directory exists and is writable");
      showErrorDialog(tr("Reversion Error"), tr("The file could not be restored from the snapshot."), errors.join("\n") );
    }else{
      qDebug() << "Reversion successful";
      QMessageBox::information(this,tr("Restore Successful"),QString(tr("The following file was succesfully restored: %1")).arg(newFilePath) );
    }
  }	  
	
}

void LPMain::openConfigGUI(){
  qDebug() << "Open Configuration UI";
	
}

void LPMain::openBackupGUI(){
  qDebug() << "Open Backup UI";
	
}

// -----------------------------------------------
//   MENU SLOTS
// -----------------------------------------------
// ==== File Menu ====
void LPMain::menuAddPool(QAction *act){
  qDebug() << "Add Pool:" << act->text();
	
}

void LPMain::menuRemovePool(QAction *act){
  qDebug() << "Remove Pool:" << act->text();
	
}

void LPMain::menuSaveSSHKey(){
  qDebug() << "Save SSH Key";
	
}

void LPMain::menuCloseWindow(){
  this->close();
}

// ==== Classic Backups Menu ====
void LPMain::menuCompressHomeDir(){
  qDebug() << "Compress Home Dir";
	
}

void LPMain::menuExtractHomeDir(){
  qDebug() << "Extract Home Dir";
	
}

// ==== Disks Menu ====
void LPMain::menuAddDisk(){
  qDebug() << "Add Disk";
	
}

void LPMain::menuRemoveDisk(){
  qDebug() << "Remove Disk";
	
}

void LPMain::menuOfflineDisk(){
  qDebug() << "Set Disk Offline";
	
}

void LPMain::menuStartScrub(){
  qDebug() << "Start Scrub";
	
}

// ==== Snapshots Menu ====
void LPMain::menuNewSnapshot(){
  qDebug() << "New Snapshot";
	
}

void LPMain::menuRemoveSnapshot(){
  qDebug() << "Remove Snapshot";
	
}
