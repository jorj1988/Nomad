#include "AssetEditor.h"
#include "MessageList.h"
#include "QMainWindow"
#include "QDockWidget"
#include "QVBoxLayout"
#include "Assets/AssetType.h"
#include "QSplitter"
#include "QToolBar"
#include "QAction"

namespace Nomad
{

namespace Editor
{

AssetEditor::AssetEditor(
    AssetType *t,
    ProjectInterface *ifc,
    AssetType::CreateInterface *c,
    UIInterface *ui)
    : _asset(t),
      _messages(nullptr)
  {
  QVBoxLayout *l = new QVBoxLayout();
  l->setContentsMargins(2, 2, 2, 2);
  l->setSpacing(4);
  setLayout(l);

  auto tools = new QToolBar(this);
  l->addWidget(tools);
  _reload = tools->addAction("Reload");
  _reload->setEnabled(false);
  connect(_reload, &QAction::triggered, [this, c]()
    {
    _asset->reload(c);
    });
  connect(t, SIGNAL(requiresReloadChanged()), this, SLOT(reloadableChanged()));

  connect(t, SIGNAL(messagesChanged()), this, SLOT(updateMessages()));

  QSplitter *splitter = new QSplitter(this);
  l->addWidget(splitter);
  splitter->setHandleWidth(4);

  if (auto e = t->createEditor(ifc, c))
    {
    QWidget *w = new QWidget(splitter);
    QVBoxLayout *vSplit = new QVBoxLayout();
    vSplit->setContentsMargins(0, 0, 0, 0);
    vSplit->setSpacing(2);
    w->setLayout(vSplit);
    splitter->addWidget(w);

    vSplit->addWidget(e);

    _messages = new MessageList(w);
    _messages->setMaximumHeight(100);
    vSplit->addWidget(_messages);
    updateMessages();
    }

  if (auto e = t->createPreview(ui, c))
    {
    splitter->addWidget(e);
    }
  }

AssetEditor::~AssetEditor()
  {
  }

AssetEditor *AssetEditor::build(
    AssetType *t,
    ProjectInterface *ifc,
    AssetType::CreateInterface *c,
    UIInterface *ui)
  {
  return new AssetEditor(t, ifc, c, ui);
  }

void AssetEditor::destroy(AssetEditor *e)
  {
  if (!e)
    {
    return;
    }

  if (auto o = qobject_cast<QDockWidget *>(e->parent()))
    {
    o->setWidget(nullptr);
    e->setParent(nullptr);
    delete o;
    }

  delete e;
  }

void AssetEditor::updateMessages()
  {
  if(!_messages)
    {
    return;
    }

  _messages->clear();
  xForeach(auto &m, _asset->messages())
    {
    auto type = m.type == AssetType::Message::Error ? MessageList::Error : MessageList::Warning;
    _messages->add(type, m.location, m.context, m.message);
    }
  }

void AssetEditor::reloadableChanged()
  {
  _reload->setEnabled(_asset->requiresReload());
  }

void AssetEditor::makeDockable(QMainWindow *mw)
  {
  if(qobject_cast<QDockWidget*>(parent()))
    {
    return;
    }

  QDockWidget *w = new QDockWidget(asset()->relativePath());
  w->setObjectName(asset()->relativePath());
  w->setAllowedAreas(Qt::AllDockWidgetAreas);

  w->setWidget(this);

  mw->addDockWidget(Qt::RightDockWidgetArea, w);
  }

void AssetEditor::showEditor()
  {
  auto tl = getTopLevel();
  tl->show();
  tl->raise();
  }

QWidget *AssetEditor::getTopLevel()
  {
  if(auto p = qobject_cast<QDockWidget*>(parent()))
    {
    return p;
    }

  return this;
  }

DynamicAssetEditor::DynamicAssetEditor(
    const QUuid &t,
    ProjectInterface *ifc,
    AssetType::CreateInterface *c,
    UIInterface *ui,
    QWidget *w)
    : QWidget(w),
      _contents(nullptr),
      _ifc(ifc),
      _cre(c),
      _ui(ui)
  {
  xAssert(_ifc);
  xAssert(_cre);
  xAssert(_ui);

  auto l = new QVBoxLayout();
  l->setContentsMargins(0, 0, 0, 0);
  setLayout(l);

  setAsset(t);
  }

void DynamicAssetEditor::setAsset(const QUuid &t)
  {
  AssetEditor::destroy(_contents);
  _contents = nullptr;

  auto handle = _ifc->getAssetHandle(t);
  if (!handle)
    {
    return;
    }

  _contents = AssetEditor::build(handle, _ifc, _cre, _ui);
  _contents->setParent(this);
  layout()->addWidget(_contents);
  }
}

}
