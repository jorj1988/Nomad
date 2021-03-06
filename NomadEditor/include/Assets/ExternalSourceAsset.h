#pragma once
#include "ExternalAssetType.h"

namespace Nomad
{

namespace Editor
{

class NOMAD_EXPORT ExternalSourceAsset : public ExternalAssetType
  {
  S_ABSTRACT_ENTITY(ExternalSourceAsset, ExternalAssetType)

public:
  ExternalSourceAsset();

  void clear() X_OVERRIDE;
  Asset *process(const QByteArray &source, CreateInterface *c) X_OVERRIDE;
  QByteArray unprocess() X_OVERRIDE;
  Asset *defaultCreate(CreateInterface *c) X_OVERRIDE;
  QWidget *createEditor(ProjectInterface *ifc, CreateInterface *c) X_OVERRIDE;

  virtual Asset *processSource(const QByteArray &source, CreateInterface *c) = 0;
  virtual QByteArray defaultSource() const = 0;

protected:
  void setSource(const QByteArray &src);
  QByteArray source() const X_OVERRIDE;

private:
  QByteArray _source;
  bool _hasSource;
  };

}

}
