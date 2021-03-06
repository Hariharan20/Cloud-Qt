#include "login_signup.h"
#include "ui_login_signup.h"
#include "account.h"
#include <QtConcurrent/QtConcurrent>
#include <QMessageBox>
#include <QDateTime>
#include <QDir>
#include <QDebug>
#include <c++/7/ios>
#include <fstream>
#include "controller.h"

void Login_SignUp::threading()
{
    Loading L;
    L.exec();
    L.show();
}

int download(QString remote,QString local)      // for files download from server
{
    QProcess::execute("skicka download "+remote+" "+local);
    return 0;
}

int upload(QString local,QString remote)    // for files upload to server
{
    QProcess::execute("skicka upload "+local+" "+remote);
    return 0;
}

int download()  // Function overload of download()
{
    QProcess::execute("skicka download pixel-database/database.dat database.dat");
    return 0;
}

int upload(QString folder_name) // Function overload of upload()
{
    QProcess::execute("skicka upload database.dat pixel-database");
    QProcess::execute("skicka mkdir pixel-database/"+QString(folder_name));
    QProcess::execute("create_files.sh "+QString(folder_name));
    QProcess::execute("skicka upload "+QString(folder_name)+" pixel-database/"+QString(folder_name));
    qDebug() << "Uploaded ";
    return 0;
}
/*

 while signing up we are downloading the required files for the current user
 with skicka files


 */

void Login_SignUp::load_friends_data()
{
    std::fstream friends_list;
    QDir::setCurrent(QDir::homePath()+"/pixel-database/"+QString(current_user));
    char friends_folder[10];
    char t[16];
    strcpy(t,current_user);
    try {
        friends_list.open("friends.txt",std::ios::in);
        if(!friends_list)
            throw(100);
        while(friends_list>>friends_folder)
        {
            QtConcurrent::run(download,QString("pixel-database/")+QString(friends_folder),QString(friends_folder));
            qDebug() << friends_folder << " Downloaded";
        }
        friends_list.close();

    }
    catch (int) {
            QMessageBox::critical(this,"File not open","Error in opening file");
    }
    QDir::setCurrent(QDir::homePath()+"/pixel-database/");
    // starts downloading friends data
}
/*
In the home page we are going to feed the user's uploads
so we are downloading our friends database.

This is achieved by download each friends data in separate threads
to increase the efficiency and speed.

*/

void Login_SignUp::stop_animation()
{
    L.movie->stop();
    L.task_completed("Completed");
    L.close();
    if(login_status==2)
    {
        load_friends_data();
        close();
        Controller Brain(current_user);
    }
    else
        show();
}

Login_SignUp::Login_SignUp(QWidget *parent) :QDialog(parent),ui(new Ui::Login_SignUp)
{
    ui->setupUi(this);

    // -------  To check internet connection -------------

    QString str = QDir::homePath();
    //
    QDir::setCurrent(str);
    QProcess::execute("online.sh");
    std::fstream f;
    int a=0;
    try {
        f.open("internet.txt",std::ios::in);
        if(!f)
            throw 100;
        f>>a;
        f.close();
        remove("internet.txt");
    } catch (int) {
       QMessageBox::critical(this,"File not open","Error in opening file");
    }

    try {
        if(a==0)
        {
            throw 400;
        }
    }
    catch (int){
        QMessageBox m;
        m.critical(this,"No Internet","Please check your internet connection");
        exit(0);
    }
    if(a!=0)
    {
        //hide();
        QString str = QDir::homePath();
        QDir working_directory;
        QDir::setCurrent(str);

        working_directory.mkpath("pixel-database");

        QDir::setCurrent(str + "/pixel-database");
        qDebug() << QDir::currentPath();
        connect(&watcher, SIGNAL(finished()), this, SLOT(stop_animation()));
        L.input_path_for_movie(":/giphy.gif");
        L.movie->start();
        QFuture<void> f2 = QtConcurrent::run(&this->L,&Loading::task_started,QString("Connecting to Server..."));
        QFuture<int> f1 = QtConcurrent::run(download);
        watcher.setFuture(f1);
    }
}

Login_SignUp::~Login_SignUp()
{
    delete ui;
}

void Login_SignUp::on_login_clicked()
{
    char input_username[16];
    char input_password[16];
    QString input_user = ui->username->text();
    QString input_pass = ui->password->text();

    // -------------- The below code converts QString to char[] ---------------

    std::strcpy(input_username,input_user.toStdString().c_str());
    std::strcpy(input_password,input_pass.toStdString().c_str());

    // ----------------------- ********* --------------------------------------
    /*
      we are opening the database in which all user and their password
      are saved
      we just check whether the input usename and password matches or not
      if matches login is successful


     */
    std::fstream fp;
    fp.open("database.dat",std::ios::ate | std::ios::binary);
    Account temp;
    fp.seekg(0,std::ios::beg);

    while(fp.read((char*)&temp,sizeof (temp)))
    {
        if(strcmp(input_username,temp.get_username())==0)
        {
            if(strcmp(input_password,temp.get_password())==0)
            {
                login_status=2;
                break;
            }
            else
            {
                login_status=1;
            }
        }
    }
    fp.close();
    if(login_status==2)
    {
        // ---------------- Login Success -------------------

        std::strcpy(current_user,input_username);
        QString str = QDir::homePath();
        QDir::setCurrent(str + "/pixel-database");
        L.input_path_for_movie(":/download.gif");
        L.movie->start();
        QFuture<void> f2 = QtConcurrent::run(&this->L,&Loading::task_started,QString("Downloading Data"));
        QFuture<int> f1 = QtConcurrent::run(download,QString("pixel-database/")+QString(input_username),QString(input_username));
        watcher.setFuture(f1);

        /*
         *
         *  Process
         *  1. download users folder
         *  2. read friends.dat start downloading all friends folder in seperate threads
         *  3. now create a merge copy of all friends folder into a feedpage folder using script
        */


    }
    else if(login_status==1)
    {
        QMessageBox::critical(this,"Incorrect Password","Check your password");
    }
    else
    {
        QMessageBox::critical(this,"Login Failed","Invaild Username or account doesnot exist");
    }
}
/*
 while creating account we check whether the user name is alredy exist and length
must exceed 2 character

 */
void Login_SignUp::on_create_clicked()
{
    char input_username[16];
    strcpy(input_username," ");
    char input_password[16];
    strcpy(input_password," ");
    QString input_user = ui->create_username->text();
    QString input_pass = ui->create_password->text();
    QString input_retype_pass = ui->create_retype_password->text();

    bool account_creation = false;

    if(input_user.count()<2 || input_user.count()>10)
    {
        QMessageBox::critical(this,"Invalid Username","Please select a username of length between 2 and 10");
    }
    else
    {
        if(input_pass.count()<2 || input_pass.count()>10)
        {
            QMessageBox::critical(this,"Invalid Password","Please select a password of length between 2 and 10");
        }
        else
        {
            if(input_pass!=input_retype_pass)
            {
                QMessageBox::critical(this,"Password Mismatch","Retype confirm password");
            }
            else
            {
                account_creation = true;
            }

        }
    }
    // -------------- The below code converts QString to char[] ---------------

    if(account_creation==true)
    {

        std::strcpy(input_username,input_user.toStdString().c_str());
        std::strcpy(input_password,input_pass.toStdString().c_str());
        // ----------------------- ********* --------------------------------------
        std::fstream reading;
        reading.open("database.dat",std::ios::ate | std::ios::binary);
        Account temp_read;
        temp_read.input(input_username,input_password);
        bool creation_status=true;
        reading.seekg(0,std::ios::beg);
        while(reading.read((char*)&temp_read,sizeof (temp_read)))
        {
            if(strcmp(input_username,temp_read.get_username())==0)
            {
                creation_status=false;
                break;
            }
        }
        reading.close();
        temp_read.input(input_username,input_password);
        if(creation_status)
        {
            std::fstream writing;
            writing.open("database.dat",std::ios::ate | std::ios::binary);
            writing.write((char*)&temp_read,sizeof (temp_read));
            writing.close();
            QMessageBox::information(this,"Created","Account created");
            L.input_path_for_movie(":/giphy.gif");
            L.movie->start();
            QFuture<void> f2 = QtConcurrent::run(&this->L,&Loading::task_started,QString("Uploading Data"));
            QFuture<int> f1 = QtConcurrent::run(upload,QString(input_username));
            watcher.setFuture(f1);
        }
        else
        {
            QMessageBox::critical(this,"Invalid Username","Username exists. Please login or retype username");
            qDebug() << QDir::currentPath();
        }
    }
}

