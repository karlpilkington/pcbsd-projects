#include "configDialog.h"
#include "ui_configDialog.h" //Qt-designer file

//Public input/output variables
/*  bool applyChanges;
    QStringList xdgOpts;
    bool keepDownloads;
    QString downloadDir;
    PBIDBAccess *DB; // Input only - current database access class
*/
ConfigDialog::ConfigDialog(QWidget* parent) : QDialog(parent), ui(new Ui::ConfigDialog){
  ui->setupUi(this); //load the Qt-Designer file
  applyChanges = FALSE;
}

ConfigDialog::~ConfigDialog(){
  delete ui;	
}

void ConfigDialog::setupDone(){
  applyChanges = FALSE; //make sure no changes by default
  //Now load the info onto the GUI
  ui->check_desktop->setChecked( xdgOpts.contains("desktop") );
  ui->check_menu->setChecked( xdgOpts.contains("menu") );
  ui->check_mime->setChecked( xdgOpts.contains("mime") );
  ui->check_paths->setChecked( xdgOpts.contains("paths") );
  ui->group_download->setChecked( keepDownloads );
  ui->line_downloadDir->setText( downloadDir.replace(QDir::homePath(),"~") );
  //Get the repo information
  repoID = DB->currentRepo();
  refreshRepoTab();
}

void ConfigDialog::refreshRepoTab(){
  DB->reloadRepoList();
  QStringList repoList = DB->availableRepos();
  int index = repoList.indexOf(repoID);
  for(int i=0; i<repoList.length(); i++){
    QStringList info = DB->repoInfo(repoList[i]);
    repoList[i].append(" - "+info[0] );	
  }
  //Now fill the repo tab
  qDebug() << "Fill combo_repo" << repoList;
  ui->combo_repo->clear();
  ui->combo_repo->addItems( repoList );
  if(!repoList.isEmpty()){
    if(index != -1){
      ui->combo_repo->setCurrentIndex(index); //will call the slot automatically
    }else{
      ui->combo_repo->setCurrentIndex(0);	  
    }	
  }
}

// === ButtonBox ===
void ConfigDialog::on_buttonBox_accepted(){
  applyChanges = TRUE; //flag that changes are available
  //generate the xdg install Options
  xdgOpts.clear();
  if(ui->check_desktop->isChecked()){ xdgOpts << "desktop"; }
  if(ui->check_menu->isChecked()){ xdgOpts << "menu"; }
  if(ui->check_mime->isChecked()){ xdgOpts << "mime"; }
  if(ui->check_paths->isChecked()){ xdgOpts << "paths"; }
  //Download Directory settings
  keepDownloads = ui->group_download->isChecked();
  downloadDir = ui->line_downloadDir->text();
  downloadDir.replace("~",QDir::homePath());
  //Repo 
  repoID = ui->combo_repo->currentText().section(" - ",0,0).simplified();
  DB->setRepo(repoID);
  //Now close the UI
  this->close();
}

void ConfigDialog::on_buttonBox_rejected(){
  applyChanges = FALSE;
  DB->setRepo(repoID); //just in case it was changed by the UI
  this->close();
}

// === Repo Tab ===
void ConfigDialog::on_combo_repo_currentIndexChanged(QString){
  //Update the repo mirror list
  
}

void ConfigDialog::on_tool_repo_add_clicked(){
  QString rpofile = QFileDialog::getOpenFileName(this,tr("Add PBI Repository"), QDir::homePath(), tr("Repository File (*.rpo)") );
  if(rpofile.isEmpty()){ return; } //cancelled
  bool ok = DB->addRepoFile(rpofile);
  if(ok){
    QMessageBox::information(this,tr("Repo Successfully Added"), tr("This repo should be ready to use in a short time (depending on your internet connection speed).") );	  
  }else{
    QMessageBox::warning(this,tr("Repo Failure"), tr("This repo could not be added.")+"\n"+ QString(tr("Please run the command '%1' manually to see the full error message.")).arg("pbi_addrepo <rpo file>") );
  }
  refreshRepoTab();
}

void ConfigDialog::on_tool_repo_remove_clicked(){
  //Get the selected repo
  QString repoName = ui->combo_repo->currentText().section(" - ",1,50).simplified();
  QString repoNum = ui->combo_repo->currentText().section(" - ",0,0).simplified();
  //Verify the removal
  if( QMessageBox::Yes == QMessageBox::question(this,tr("Verify Removal"),repoName+"\n\n"+tr("Are you sure you wish to remove this PBI repository?"),QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel) ){
    //remove the repo
    bool ok = DB->removeRepo(repoNum);
    if(!ok){
      QMessageBox::warning(this,tr("Repo Failure"), tr("This repo could not be removed.")+"\n"+ QString(tr("Please run the command '%1' manually to see the full error message.")).arg("pbi_deleterepo "+repoNum) );
    }
    refreshRepoTab();
  }
}

void ConfigDialog::on_tool_repomirror_add_clicked(){
	
}

void ConfigDialog::on_tool_repomirror_remove_clicked(){
	
}

void ConfigDialog::on_tool_repomirror_up_clicked(){
	
}

void ConfigDialog::on_tool_repomirror_down_clicked(){
	
}
	
// === Config Tab ===
void ConfigDialog::on_group_download_toggled(bool checked){
  ui->frame_dldir->setVisible(checked);
}

void ConfigDialog::on_tool_getDownloadDir_clicked(){
  QString dirpath = QFileDialog::getExistingDirectory(this, tr("Select Download Directory"), QDir::homePath());
  if(dirpath.isEmpty()){return;} //not cancelled
  dirpath.replace(QDir::homePath(),"~");
  ui->line_downloadDir->setText(dirpath);
}
