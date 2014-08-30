#include "Assets/ExternalDbAsset.h"
#include "shift/TypeInformation/spropertyinformationhelpers.h"
#include "NAsset.h"
#include "Application.h"
#include "TextEditor.h"

namespace Nomad
{

namespace Editor
{

S_IMPLEMENT_PROPERTY(ExternalDbAsset, Nomad)

void ExternalDbAsset::createTypeInformation(
    Shift::PropertyInformationTyped<ExternalDbAsset> *info,
    const Shift::PropertyInformationCreateData &data)
  {
  auto childBlock = info->createChildrenBlock(data);
  }

void ExternalDbAsset::clear()
  {
  auto asset = cachedAsset();
  if (auto parent = asset->parent()->castTo<Shift::Set>())
    {
    parent->remove(asset);
    }
  }

Asset *ExternalDbAsset::process(Shift::Set *s, const QByteArray &src)
  {
  auto asset = Application::loadSource(src, s);
  if(!asset)
    {
    return nullptr;
    }

  return asset->castTo<Asset>();
  }

QByteArray ExternalDbAsset::unprocess(Asset *)
  {
  return source();
  }

QWidget *ExternalDbAsset::createEditor()
  {
  class Editor : public TextEditor
    {
  public:
    Editor(const QString &s, ExternalDbAsset *ass)
        : TextEditor(s),
          _asset(ass)
      {
      connect(
        this,
        &QTextEdit::textChanged,
        [this]()
          {
          auto asset = _asset->cachedAsset();
          if (!asset)
            {
            return;
            }

          if (auto parent = asset->parent()->castTo<Shift::Set>())
            {
            _asset->clear();
            _asset->initialiseFromSource(parent, toPlainText().toUtf8());
            }
          }
        );
      }

  private:
    ExternalDbAsset *_asset;
    };

  return new Editor(source(), this);
  }

QByteArray ExternalDbAsset::source()
  {
  auto ass = cachedAsset();
  if(!ass)
    {
    return QByteArray();
    }

  return Application::toSource(ass);
  }
}

}
