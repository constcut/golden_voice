import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15
import QtQuick.Dialogs 1.1
import QtQuick.Controls 1.4 as Quick1


Item {

    id: diaryCardItem

    function updateFields()
    {
        var allFields = cardEngine.getAllGroupFields(groupsNames.currentText)
        fieldsRepeater.model = 0
        fieldsNames.model = allFields
        fieldsRepeater.fieldsNames = allFields
        fieldsRepeater.model = allFields.length
    }

    function updateCard()
    {
        cardName.text = cardEngine.getCardName()
        cardDescription.text =  cardEngine.getCardDescription()
        groupsNames.model = cardEngine.getAllGroupsNames()

        updateFields()
    }


    Component.onCompleted:
    {
        var allCardsNames = sqlBase.getAllCardsNames()
        cardCombo.model = allCardsNames
        mergeCardCombo.model = allCardsNames

        updateCard()
    }

    Dialog
    {
        id: calendarDialog

        width: 400
        height: 400

        Quick1.Calendar
        {
            anchors.fill: parent

            id: calendar

            onSelectedDateChanged:
            {
                dateText.text = selectedDate
                dateText.text = dateText.text.substring(0, 10)
            }
        }
    }


    ColumnLayout
    {
        spacing: 10
        y: 40
        x: 40


        RowLayout
        {
            spacing: 10


            ComboBox
            {
                id: cardCombo
                onCurrentTextChanged: {
                    var jsonCard = sqlBase.getCardJSON(currentText)
                    cardEngine.parseJSON(jsonCard)

                    diaryCardItem.updateCard()
                }
            }

            ComboBox {
                id: mergeCardCombo
            }

            Button
            {
                text: "merge"

                onClicked:
                {
                    if (mergeCardCombo.currentText !== cardCombo.currentText)
                    {
                        var jsonCard = sqlBase.getCardJSON(mergeCardCombo.currentText)
                        cardEngine.mergeJSON(jsonCard)
                        diaryCardItem.updateCard()
                    }
                }
            }

            RoundButton {
                text: "To card records"
                onClicked: mainWindow.requestCardRecords()
            }
        }



        RowLayout
        {
            spacing: 10

            Text {
                id: cardName
            }

            Text {
                id: cardDescription
            }

            Text {
                text: "Group: "
            }

            ComboBox {
                id: groupsNames
                implicitWidth: 250

                onCurrentTextChanged: {
                    diaryCardItem.updateFields()
                }
            }

            Text {
                text: "Field: "
            }

            ComboBox {
                id: fieldsNames
                implicitWidth: 300
            }

            Text {
                text: "Date: "
            }
            Text {
                id: dateText
            }
            RoundButton {
                text: ".."
                onClicked: calendarDialog.open()
            }
            RoundButton { //Just and example on days rotations (used if filling old dates)
                text : "<"
                onClicked: {
                    var theDay = calendar.selectedDate
                    theDay.setDate(theDay.getDate() - 1);
                    calendar.selectedDate = theDay
                }
                visible: false
            }
            RoundButton { //Just and example on days rotations (used if filling old dates)
                text: ">"
                onClicked: {
                    var theDay = calendar.selectedDate
                    theDay.setDate(theDay.getDate() + 1);
                    calendar.selectedDate = theDay
                }
                visible: false
            }

            Button
            {
                text: "Add values"

                onClicked:
                {
                    var allGroupFields = []

                    for (var i = 0; i < fieldsRepeater.model; ++i)
                    {
                        var fieldInfo = fieldsRepeater.itemAt(i).getFieldInfo()
                        allGroupFields.push(fieldInfo)
                    }

                    sqlBase.addCardRecord(cardCombo.currentText, dateText.text,
                                          groupsNames.currentText, allGroupFields)

                    diaryCardItem.updateFields()
                    notifyAddedDialog.open()
                }

            }
        }

        Dialog {
            id: notifyAddedDialog
            width: 150
            height: 100
            Text {
                text: "Record was added"
            }
            x: diaryCardItem.width/2 - width/2
            y: diaryCardItem.height/2 - height/2
        }


        IntValidator {
            id: intValidator
        }

        DoubleValidator {
            id: realValidator
        }


        ColumnLayout //RowLayout
        {

            Repeater
            {
                id: fieldsRepeater

                property var fieldsNames: []

                Rectangle
                {
                    id: fieldRectangle

                    width: textInfo.width + 10 + textField.width
                    height: 40

                    property string fieldType: ""
                    property string fieldName: ""

                    function getFieldInfo()
                    {
                        var list = []
                        list.push(fieldName)
                        list.push(fieldType)

                        var isEmpty = false

                        if (fieldType == "text" || fieldType == "int" || fieldType == "real")
                        {
                            list.push(textField.text)
                            if (textField.text === "")
                                isEmpty = true
                        }

                        if (fieldType == "bool")
                        {
                            if (checkField.checked)
                                list.push(1)
                            else
                            {
                                list.push(0)
                                isEmpty = true
                            }
                        }

                        var groupName = groupsNames.currentText

                        if (fieldType == "enum")
                        {
                            var enumName = cardEngine.getFieldEnum(groupName, fieldName)
                            var enumValues = cardEngine.getEnumValues(enumName)
                            list.push(enumValues[comboField.currentIndex])
                            //Maybe for optimization can leave comboField.currentIndex and translate in sqlbase on add DCR

                            if (comboField.currentIndex === 0)
                                isEmpty = true
                        }

                        if (fieldType == "range")
                        {
                            var rangeMin = cardEngine.getFieldRangeMin(groupName, fieldName)
                            var rangeValue = comboField.currentIndex + rangeMin
                            list.push(rangeValue)

                            if (rangeValue == 0)
                                isEmpty = true
                        }

                        list.push(isEmpty)

                        return list
                    }

                    Component.onCompleted:
                    {

                        var fieldName = fieldsRepeater.fieldsNames[index]
                        var groupName = groupsNames.currentText

                        var fieldType = cardEngine.getFieldType(groupName, fieldName)
                        var fieldDescription = cardEngine.getFieldDescription(groupName, fieldName)

                        fieldRectangle.fieldType = fieldType
                        fieldRectangle.fieldName = fieldName

                        textInfo.text = fieldName
                        toolTip.text = fieldDescription

                        textField.visible = (fieldType === "text") || (fieldType === "int") || (fieldType === "real")
                        comboField.visible = (fieldType === "range") || (fieldType === "enum")
                        checkField.visible = fieldType === "bool"

                        if (textField.visible) {
                            textField.placeholderText = fieldName
                            textInfo.text = ""
                        }

                        if (checkField.visible) {
                            checkField.text = fieldName
                            textInfo.text = ""
                        }

                        if (fieldType === "int")
                            textField.validator = intValidator

                        if (fieldType === "real")
                            textField.validator = realValidator

                        if (fieldType === "range")
                        {
                            var rangeMin = cardEngine.getFieldRangeMin(groupName, fieldName)
                            var rangeMax = cardEngine.getFieldRangeMax(groupName, fieldName)

                            var rangeList = []

                            for (var i = rangeMin; i <= rangeMax; ++i)
                                rangeList.push(i)

                            comboField.model = rangeList
                        }

                        if (fieldType === "enum")
                        {
                            var enumName = cardEngine.getFieldEnum(groupName, fieldName)
                            var displayNames = cardEngine.getEnumDisplayNames(enumName)
                            comboField.model = displayNames
                        }

                    } //Component.onCompleted



                    Text
                    {
                        y: 10
                        id: textInfo
                    }

                    TextField
                    {
                        id: textField
                        x: textInfo.width + 10

                    }
                    ComboBox {
                        id: comboField
                        x: textInfo.width + 10
                    }
                    CheckBox {
                        id: checkField
                        x: textInfo.width + 10

                        ToolTip {
                           id: toolTip
                           visible: checkField.hovered && text !== ""
                           text: ""
                        }
                    }
                }

            } //Repater

        } //RowLayout
    }


    function keyboardEventSend(key, mode) {
        //Заглушка обработки клавиш
    }

}
