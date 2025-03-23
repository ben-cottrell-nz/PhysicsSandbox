import QtQuick 2.15
import Extras 1.0

Item {
    width: 512
    height: 512

    SkiaStar { anchors.fill: parent }

    Text {
        text: "FPS: ?, Objects: ?"
        color: Qt.black
        font.pointSize: 26
    }
}
