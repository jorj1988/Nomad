import "../Eks/EksBuild" as Eks;

Eks.Application {
  name: "NomadEditor"
  toRoot: "../"

  cpp.includePaths: base.concat([ "." ])

  files: [ "**/*.h", "**/*.cpp", "**/*.ui" ]

  Depends { name: "Qt.gui"}
  Depends { name: "Qt.widgets" }

  Depends { name: "Eks3D" }
  Depends { name: "EksGui" }
  Depends { name: "NomadCore" }
  Depends { name: "ShiftCore" }
}
