import QtQuick 2.15
import Extras 1.0

Item {
    width: 512
    height: 512

    SkiaStar {
        id: skiaStar
        width: 512; height: 512
        SequentialAnimation {
            running: true
            NumberAnimation { target: skiaStar; property: "x"; to: 50; duration: 500 }
            NumberAnimation { target: skiaStar; property: "y"; to: 50; duration: 500 }
            loops: Animation.Infinite
        }
    }

    Text {
        text: "FPS: ?, Objects: ?"
        color: Qt.black
        font.pointSize: 26
    }
}
