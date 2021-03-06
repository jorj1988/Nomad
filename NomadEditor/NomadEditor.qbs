import "../Eks/EksBuild" as Eks;

Eks.Library {
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
  Depends { name: "ShiftQt" }

  Export {
    Depends { name: "cpp" }

    cpp.includePaths: [ "./include" ]
  }
}
