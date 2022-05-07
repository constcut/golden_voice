import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.1
import QtQuick.Controls 1.4 as Quick1


Item {

    id: testsEngineItem

    Component.onCompleted:
    {
        allTests.model = sqlBase.getAllTestsNames()

        loadTest()
    }

    function restartEngine()
    {
        var testJson = sqlBase.getTestJSON(allTests.currentText)
        testsEngine.parseJSON(testJson)
        testsEngineItem.loadTest()
    }

    function loadTest()
    {
        testsEngineItem.questionNumber = 0
        testName.text = testsEngine.getName()
        questionsCount.text = testsEngine.questionsCount()
        loadQuestion()
    }

    property int questionNumber: 0

    function loadQuestion()
    {
        var qNum = testsEngineItem.questionNumber
        questionNumber.text = qNum
        questionText.text = testsEngine.getQuestionText(qNum)

        var optionsCount = testsEngine.getOptionsCount(qNum)
        var questionType = testsEngine.getQuestionType(qNum)

        checksRepeater.model = 0
        radioRepeater.model = 0
        questionField.visible = false

        var optionsTexts = testsEngine.getOptionsTexts(qNum)
        var answers = testsEngine.getAnswers(qNum)
        var i = 0
        var j = 0

        if (questionType === "radio")
        {
            radioRepeater.model = optionsCount

            for (i = 0; i < optionsCount; ++i)
            {
                radioRepeater.itemAt(i).text = optionsTexts[i]

                for (j = 0; j < answers.length; ++j)
                    if (answers[j] === optionsTexts[i])
                        radioRepeater.itemAt(i).checked = true

            }
        }
        else if (questionType === "check")
        {
            checksRepeater.model = optionsCount

            for (i = 0; i < optionsCount; ++i)
            {
                checksRepeater.itemAt(i).text = optionsTexts[i]

                for (j = 0; j < answers.length; ++j)
                    if (answers[j] === optionsTexts[i])
                        checksRepeater.itemAt(i).checked = true
            }
        }
        else if (questionType === "text")
        {
            questionField.visible = true
            if (answers.length === 1)
                questionField.text = answers[0]
        }

    }

    function loadNextQuestion()
    {
        var qCount = testsEngine.questionsCount()
        if (testsEngineItem.questionNumber + 1 < qCount)
        {
            testsEngineItem.questionNumber += 1
            testsEngineItem.loadQuestion()
        }
    }

    function loadPrevQuestion()
    {
        if (testsEngineItem.questionNumber > 0)
        {
            testsEngineItem.questionNumber -= 1
            testsEngineItem.loadQuestion()
        }
    }


    ColumnLayout
    {
        spacing: 10
        y: 40
        x: 40

        RowLayout {
            spacing: 10

            ComboBox {
                id: allTests
                implicitWidth: 150

                onCurrentTextChanged: {
                    testsEngineItem.restartEngine()
                }
            }

            RoundButton {
                text: "Tests results"
                onClicked: mainWindow.requestTestsResults()
            }
        }

        RowLayout
        {
            spacing: 10

            Text {
                text: "Test name: "
            }
            Text {
                id: testName
            }
            Text {
                text: "Questions count: "
            }
            Text {
                id: questionsCount
            }
            Text {
                text: "Questions left"
            }
            Text {
                id: questionsLeft
            }
            Text {
                text: "Current rate"
            }
            Text {
                id: currentRate
            }

            RoundButton {
                text: "Prev"
                onClicked: testsEngineItem.loadPrevQuestion()
            }
            RoundButton {
                text: "Next"
                onClicked: testsEngineItem.loadNextQuestion()
            }
        }

        Rectangle
        {
            width: 700
            height: 400 //Calculate size and probably use flick
            border.color: "lightgreen"

            ColumnLayout
            {
                spacing:  20

                y: 10
                x: 20

                Text {
                    id: questionNumber
                }
                Text {
                    id: questionText
                }

                Repeater
                {
                    id: checksRepeater
                    CheckBox {
                        id: checkOption
                    }
                }
                Repeater
                {
                    id: radioRepeater
                    RadioButton {
                        id: radioOption
                    }
                }
                TextField
                {
                    id: questionField
                    placeholderText: "Answer"
                }
                RoundButton
                {
                    text: "Save answer"
                    onClicked: {
                        var qNum = testsEngineItem.questionNumber
                        var questionType = testsEngine.getQuestionType(qNum)

                        var i

                        if (questionType === "check")
                        {
                            var checkAnswers = []

                            for (i = 0; i < checksRepeater.model; ++i)
                                if (checksRepeater.itemAt(i).checked)
                                    checkAnswers.push(checksRepeater.itemAt(i).text)

                            testsEngine.answerCheckQuestion(qNum, checkAnswers);
                        }
                        else if (questionType === "radio")
                        {
                            for (i = 0; i < radioRepeater.model; ++i)
                                if (radioRepeater.itemAt(i).checked)
                                {
                                    testsEngine.answerQuestion(qNum, radioRepeater.itemAt(i).text);
                                    break;
                                }
                        }
                        else if (questionType === "text")
                            testsEngine.answerQuestion(qNum, questionField.text)


                        questionsLeft.text = testsEngine.questionsLeft()
                        currentRate.text = testsEngine.testCurrentRate()

                        if (testsEngine.isTestFinished() === false)
                            testsEngineItem.loadNextQuestion()
                        else
                        {
                            var rateName = testsEngine.getRateName()
                            var rateDescription = testsEngine.getRateDescription()

                            sqlBase.addTestResult(allTests.currentText, currentRate.text, rateName)
                            testsEngineItem.restartEngine()

                            testResult.text = rateName
                            resultDescription.text = rateDescription
                            resultDialog.open()
                        }
                    }
                }
            } //ColumnLayout

        } //Rectangle

    } // main ColumnLayout

    Dialog
    {
        x: testsEngineItem.width / 2 - width / 2
        y: testsEngineItem.height / 2 - height / 2

        id: resultDialog
        width: 400
        height: 200
        ColumnLayout
        {
            Text {
                text: "Test result:"
            }
            Text {
                id: testResult
            }
            Text {
                text: "Result description: "
            }
            Text {
                id: resultDescription
            }
        }
    }


    function keyboardEventSend(key, mode) {
        //Заглушка обработки клавиш
    }

}
