*** Settings ***
Library         String
Library         SerialLibrary

*** Variables ***
${com}              COM9
${baud}             115200
${board}            nRF5340
${seq_1}            000003X
${expected_1}       3X

${seq_2}            000004X
${expected_2}       4X

${seq_3}            000002X
${expected_3}       2X

${bad_seq}          001067X
${expected_err}     -3X

*** Test Cases ***
Connect Serial
    Log To Console    Connecting to ${board} on ${com}
    Add Port    ${com}    baudrate=${baud}    encoding=ascii
    Port Should Be Open    ${com}
    Reset Input Buffer
    Reset Output Buffer

Test Red LED (First Command)
    Write Data    ${seq_1}    encoding=ascii
    Log To Console    Sent time for Red LED: ${seq_1}
    ${read} =    Read Until    \n    encoding=ascii
    ${read} =    Strip String    ${read}
    Should Be Equal As Strings    ${read}    ${expected_1}
    Log To Console    Red LED test PASSED!

Test Green LED (Second Command)
    Write Data    ${seq_2}    encoding=ascii
    Log To Console    Sent time for Green LED: ${seq_2}
    ${read} =    Read Until    \n    encoding=ascii
    ${read} =    Strip String    ${read}
    Should Be Equal As Strings    ${read}    ${expected_2}
    Log To Console    Green LED test PASSED!

Test Yellow LED (Third Command)
    Write Data    ${seq_3}    encoding=ascii
    Log To Console    Sent time for Yellow LED: ${seq_3}
    ${read} =    Read Until    \n    encoding=ascii
    ${read} =    Strip String    ${read}
    Should Be Equal As Strings    ${read}    ${expected_3}
    Log To Console    Yellow LED test PASSED!

Test Invalid Time String
    Write Data    ${bad_seq}    encoding=ascii
    Log To Console    Sent invalid time: ${bad_seq}
    ${read} =    Read Until    \n    encoding=ascii
    ${read} =    Strip String    ${read}
    Should Be Equal As Strings    ${read}    ${expected_err}
    Log To Console    Invalid time test PASSED!

Disconnect Serial
    Log To Console    Disconnecting ${board}
    [Teardown]    Delete Port    ${com}