#!/bin/bash
#
# startup script template that setups environment variables
# builds the TartsWebClient
# and then starts the TartsWebClient
export JOTTAI_API_HOST=http://REPLACE_WITH_JOTTAI_API_HOST/jottai/api/sensor-data
export JOTTAI_API_KEY=REPLACE_WITH_JOTTAI_API_KEY
export JOTTAI_ID=REPLACE_WITH_JOTTAI_BOT_ID
export GATEWAY_ID=REPLACE_WITH_TARTS_GATEWAY_ID
./build.sh
./TartsWebClient
