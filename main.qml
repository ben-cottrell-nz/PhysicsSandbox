import QtQuick 2.15
import QtQuick.Controls.Basic 2.15
import Extras 1.0

Item {
    width: 512
    height: 512

    PhysWorldView {
        id: physWorldView
        anchors.fill: parent
    }
    Button {
        onClicked:  physWorldView.testAddBoxes();
        text: "Run Test: \"Add Boxes\""
    }
}
