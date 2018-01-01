#!/bin/bash
#
# startup script template that setups environment variables
# builds the TartsWebClient
# and then starts the TartsWebClient
export YOG_API_HOST=http://REPLACE_WITH_YOG_ROBOT_API_HOST/yog-robot/api/sensor-data
export YOG_API_KEY=REPLACE_WITH_YOG_ROBOT_API_KEY
export YOG_BOT_ID=REPLACE_WITH_YOG_ROBOT_BOT_ID
export GATEWAY_ID=REPLACE_WITH_TARTS_GATEWAY_ID
./build.sh
./TartsWebClient
