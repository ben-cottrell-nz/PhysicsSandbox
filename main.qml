import QtQuick 2.15
import Extras 1.0

Item {
    width: 512
    height: 512

    SkiaStar { width: 256; height: 256 }

    Text {
        text: "FPS: ?, Objects: ?"
        color: Qt.black
        font.pointSize: 26
    }
}
