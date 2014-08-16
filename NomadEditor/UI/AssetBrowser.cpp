#include "AssetBrowser.h"
#include "ui_AssetBrowser.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"
#include "shift/Properties/sdata.inl"
#include "shift/Serialisation/sloader.h"
#include "shift/Serialisation/sjsonio.h"
#include "Application.h"
#include "NewAsset.h"
#include "NAsset.h"
#include "Assets/NShader.h"
#include "QFileSystemModel"
#include "QMessageBox"
#include "QFile"

namespace Nomad
{

namespace Editor
{

S_IMPLEMENT_PROPERTY(AssetBrowserData, NomadEditor)

void AssetBrowserData::createTypeInformation(
    Shift::PropertyInformationTyped<AssetBrowserData> *info,
    const Shift::PropertyInformationCreateData &data)
  {
  auto block = info->createChildrenBlock(data);

  block.add(&AssetBrowserData::manager, "manager");
  }

AssetBrowserData::AssetBrowserData()
    : _paths(Eks::Core::defaultAllocator())
  {
  }

Nomad::Asset *AssetBrowserData::load(const char *name, Shift::Array *parent)
  {
  QFile toLoad(name);
  if(!toLoad.open(QFile::ReadOnly))
    {
    xAssertFail(name);
    return nullptr;
    }

  qDebug() << "loading file" << name;

  Eks::String toLoadStr = toLoad.readAll().data();

  Shift::LoadBuilder builder;
  Eks::TemporaryAllocator alloc(parent->temporaryAllocator());
  auto loading = builder.beginLoading(parent, &alloc);

  Shift::JSONLoader loader;
  loader.load(&toLoadStr, &builder);

  xAssert(loading->loadedData().size() == 1);
  if (loading->loadedData().size() != 1)
    {
    return nullptr;
    }

  auto asset = loading->loadedData().front()->castTo<Nomad::Asset>();
  manager.registerAsset(asset);
  _paths[asset->uuid()] = name;

  return asset;
  }

Asset *AssetBrowserData::load(const QUuid &name, Shift::Array *parent)
  {
  QString path = name.toString();

  auto it = _paths.find(name);
  if (it == _paths.end())
    {
    qWarning() << "Can't find asset for " << name;
    return nullptr;
    }

  return load(it->first, parent);
  }

bool AssetBrowserData::requiresReload(const QUuid &)
  {
  return false;
  }

Asset *AssetBrowserData::createAsset(const Shift::PropertyInformation *info, const QString &location)
  {
  Asset *a = manager.createAsset(info);
  _paths[a->uuid()] = location;

  saveAsset(a);

  return a;
  }

void AssetBrowserData::saveAsset(Asset *a)
  {
  xAssert(_paths.contains(a->uuid()), a->uuid().toByteArray().data());
  QFile file(_paths[a->uuid()]);
  file.open(QFile::WriteOnly);

  Shift::SaveBuilder builder;
  Shift::JSONSaver writer;
  writer.setAutoWhitespace(true);

  Eks::String fileStr;
    {
    auto block = writer.beginWriting(&fileStr);

    builder.save(a, true, &writer);
    }

  file.write(fileStr.data());
  }

void AssetBrowserData::loadAsset(const QString &d)
  {
  load(d.toUtf8().data(), manager.assetParent());
  }

bool AssetBrowserData::hasLoaded(const QString &d) const
  {
  QFileInfo a(d);
  xForeach(auto v, _paths.values())
    {
    QFileInfo b(v);
    if (a == b)
      {
      return true;
      }
    }

  return false;
  }

AssetBrowser::AssetBrowser(ProjectInterface *ifc, QWidget *parent) :
  QDockWidget(parent),
  _project(ifc),
  _ui(new Ui::AssetBrowser)
  {
  _ui->setupUi(this);

  _project->addProjectChanged(this, SLOT(setupProject()));

  auto scratch = _project->getScratchParent();
  _browser = scratch->add<AssetBrowserData>();

  _model = new QFileSystemModel;

  setupProject();
  }

AssetBrowser::~AssetBrowser()
  {
  delete _ui;
  }

void AssetBrowser::setupProject()
  {
  auto currentProject = _project->getCurrentProject();
  _ui->dockWidgetContents->setEnabled(currentProject != nullptr);
  _ui->treeView->setModel(nullptr);

  if(!currentProject)
    {
    return;
    }

  _browser->manager.reset(_browser);

  QFileInfo project = currentProject->path().toQString();
  auto root = project.dir().absolutePath();
  _model->setRootPath(root);
  _model->setNameFilters(QStringList() << ASSET_FILTER);
  connect(_model, SIGNAL(rowsInserted(QModelIndex,int,int)), this, SLOT(fileAdded(QModelIndex,int,int)));

  _ui->treeView->setModel(_model);
  _ui->treeView->setRootIndex(_model->index(root));

  loadAllAssets(root);
  rebuildUI();

  connect(_ui->treeView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(openAsset(const QModelIndex &)));
  connect(_ui->add, SIGNAL(clicked()), this, SLOT(addAsset()));
  connect(_ui->remove, SIGNAL(clicked()), this, SLOT(removeAsset()));
  }

void AssetBrowser::rebuildUI()
  {
  }

void AssetBrowser::openAsset(const QModelIndex &i)
  {
  qDebug() << _model->filePath(i);
  }

void AssetBrowser::addAsset()
  {
  auto f = new NewAsset(this);

  QFileInfo info(_project->getCurrentProject()->path().data());
  QString path = info.dir().absolutePath();

  auto rows = _ui->treeView->selectionModel()->selectedRows();
  if (rows.size() > 0 && _model->isDir(rows[0]))
    {
    path = _model->filePath(rows[0]);
    qDebug() << path;
    }

  if (f->show(path) != QDialog::Accepted)
    {
    return;
    }

  _browser->createAsset(f->type(), f->location());
  }

void AssetBrowser::removeAsset()
  {
  auto rows = _ui->treeView->selectionModel()->selectedRows();

  if (QMessageBox::question(
        nullptr,
        "Delete Assets",
        tr("Are you sure you want to delete %n asset(s)?", 0, rows.size()),
        QMessageBox::Yes,
        QMessageBox::No) != QMessageBox::Yes)
    {
    return;
    }

  xForeach (auto r, rows)
    {
    QFile f(_model->filePath(r));

    f.remove();
    }
  }

void AssetBrowser::fileAdded(const QModelIndex &parent, int first, int last)
  {
  for (int i = first; i <= last; ++i)
    {
    QModelIndex index = parent.child(i, 0);
    QString path = _model->filePath(index);

    QFileInfo info(path);
    if (info.suffix() != ASSET_EXT)
      {
      continue;
      }

    if (!_browser->hasLoaded(path))
      {
      _browser->loadAsset(path);
      }
    }
  }

void AssetBrowser::loadAllAssets(const QString &dir)
  {
  xForeach(auto file, QDir(dir).entryList(
      QStringList() << ASSET_FILTER,
      QDir::Files|QDir::AllDirs|QDir::NoDotAndDotDot))
    {
    QString path = dir + "/" + file;
    if (QFileInfo(path).isDir())
      {
      loadAllAssets(path);
      continue;
      }

    _browser->loadAsset(path);
    }
  }
}

}
