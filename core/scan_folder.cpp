#include "scan_folder.hpp"

#include <QDebug>
#include <QDirIterator>
#include <QFileInfo>
#include <QStringList>

#include <opencv2/highgui.hpp>

#include <set>

scan_folder::scan_folder(QStringList const &abs_dirs,
                         QStringList const &scan_types,
                         bool scan_subfolders,
                         QObject *parent) :
    QThread(parent),
    abs_dirs_(abs_dirs),
    scan_subfolders_(scan_subfolders),
    scan_types_(scan_types)
{

}

QStringList scan_folder::get_abs_file_path() const
{
    return  files_;
}

/**
 * @brief Exclude child folders if any
 * @param folders self explained
 * @return folders without any child folders
 * @code
 * "c:/users/wahaha/pics/konosuba"
 * "c:/users/wahaha/pics/konosuba/first_episode"
 * "c:/users/wahaha/pics/konosuba/sec_episode"
 *
 * There are two child folders of konosuba, they
 * are first episode and second episode, this function
 * will remove these child folders.
 */
QStringList scan_folder::exclude_child_folders(const QStringList &folders) const
{
    //TODO : find a better, more efficient, easier to understand solution
    QStringList result = folders;
    result.sort();
    for(int i = 0; i < result.size() - 1; ++i){
        QStringList exclude_fd;
        for(int k = 0; k <= i; ++k){
            exclude_fd.push_back(result[k]);
        }
        for(int j = i; j != result.size(); ++j){
            if(!result[j].startsWith(result[i])){
                exclude_fd.push_back(result[j]);
            }
        }
        result = exclude_fd;
    }

    return result;
}

void scan_folder::scan_folders()
{
    files_.clear();
    if(scan_subfolders_){
        abs_dirs_ = exclude_child_folders(abs_dirs_);
    }

    struct icp
    {
        bool operator()(QString const &lhs, QString const &rhs) const
        {
            return lhs.compare(rhs, Qt::CaseInsensitive) > 0;
        }
    };

    std::set<QString, icp> is_img;
    for(auto const &type : scan_types_){
        is_img.insert(type);
    }

    emit progress("Total files found : 0");
    for(auto const &dir : abs_dirs_){
        auto const iter_flags = scan_subfolders_ ?
                QDirIterator::Subdirectories : QDirIterator::NoIteratorFlags;
        QDirIterator directories(dir,
                                 QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot,
                                 iter_flags);
        while(directories.hasNext()){
            //qDebug()<<directories.fileInfo().suffix();
            if(is_img.find(directories.fileInfo().suffix()) != std::end(is_img)){
                //qDebug()<<"can find suffix";
                files_.push_back(directories.filePath());
                emit progress(QString("Total files found : %1").
                              arg(files_.size()));
            }else{
                //qDebug()<<"cannot find suffix";
            }
            directories.next();
        }
    }
    //qDebug()<<files_;

    emit end();
}

void scan_folder::run()
{    
    scan_folders();
}
