import QtQuick 2.12
import diaryth 1.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.1

Item {
    id: item

    property string filename: "default"

    function reloadFile() {

    }

    ScrollView
    {
        width: parent.width
        height: 410

        Flickable
        {
            id: flick
            y: 5
            x: 0
            width: parent.width
            height: 410
            contentWidth: 3000
            contentHeight:  parent.height
            property int pressedX : 0

            MouseArea
            {
                x:0
                y:20
                width: parent.width
                height: 400

                onPressed: {
                    flick.pressedX = mouseX
                }
                onReleased: {
                    var diff =  flick.pressedX - mouseX
                    flick.contentX += diff
                    if (flick.contentX < 0)
                        flick.contentX = 0
                }
                onClicked:{
                    var minRmStep = waveShape.getMinRmsStep()
                    waveShape.setWindowPosition(mouseX * minRmStep/2.0)
                    //acgraph.loadByteArray(waveShape.getPCM(mouseX * minRmStep / 2.0, 4096));
                    acgraph.loadFloatSamples(waveShape.getFloatSamples(mouseX * minRmStep / 2.0, 4096))
                    yinInfo.text = acgraph.getLastFreq()
                            + "\nTime = " + ((mouseX * minRmStep / 2.0) / 44100.0).toFixed(4)
                            //+ "\nSpecPitch= " + spectrum.getSpectrumF0().toFixed(3)
                    //spectrum.loadFloatSamples(acgraph.getACF())

                }
                onDoubleClicked: {
                    var minRmStep = waveShape.getMinRmsStep()
                    audio.loadWindowPCM(waveShape.getPCM(mouseX * minRmStep / 2.0, 4096))
                    audio.startPlayback()
                }

            }

            VisualReport
            {
                id: visualReport
                height:  parent.height
                width: 3000
                y: 5
                Component.onCompleted: {

                }
            }

        }
    }

    function keyboardEventSend(key, mode) {
        //Заглушка, но можно реализовать логику здесь
    }

}
