import QtQuick 2.12
import diaryth 1.0
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.1


Item {

    id: fullHDReport


    Component.onCompleted:
    {

    }


    JsonReport
    {
        id: jsonReport

        Component.onCompleted:
        {
            flick.contentWidth = jsonReport.getFullWidth()

            //visualReport1.width = jsonReport.getFullWidth()
            //visualReport2.width =  jsonReport.getFullWidth()
            //chunkId.model = jsonReport.getChunksCount()

            reportsRepeater.itemAt(0).setParent(jsonReport)
            reportsRepeater.itemAt(1).setParent(jsonReport)
            reportsRepeater.itemAt(2).setParent(jsonReport)
            reportsRepeater.itemAt(3).setParent(jsonReport)

            reportsRepeater.itemAt(0).setType(VisualTypes.PlainWords)
            reportsRepeater.itemAt(1).setType(VisualTypes.PraatInfo)
            reportsRepeater.itemAt(2).setType(VisualTypes.ReportFields)
            reportsRepeater.itemAt(3).setType(VisualTypes.Pitch)
        }

    }


    property var reportsHeight: [70, 200, 200, 500]

    function calculateY(idx)
    {
        var hSum = 0
        for (var i = 0; i < idx; ++i)
        {
            hSum += reportsHeight[i - 1]
        }

        return hSum + 5 * (idx + 1)
    }


    ScrollView
    {
        id: scroll
        width: parent.width
        height: fullHDReport.reportsHeight[0] + reportsHeight[1] + reportsHeight[2] + reportsHeight[3]

        Flickable
        {
            id: flick
            y: 5
            x: 0
            width: parent.width
            height: scroll.height
            contentWidth: 3000
            contentHeight:  parent.height
            property int pressedX : 0


            Repeater
            {
                id: reportsRepeater

                model: 4

                VisualReport
                {
                    id: visualReport
                    height:  fullHDReport.reportsHeight[index]
                    width: 3000
                    y:  fullHDReport.calculateY(index)

                    MouseArea
                    {
                        anchors.fill: parent
                        onClicked:{

                        }
                        onDoubleClicked: {
                            var idx = jsonReport.eventIdxOnClick(mouseX, mouseY)
                            jsonReport.selectEvent(idx)
                        }

                    }
                }


            }




        } //Flickable

    } //ScrollView




}
