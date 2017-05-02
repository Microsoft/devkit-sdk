'use strict';
// This function is triggered each time a message is revieved in the IoTHub.
// The message payload is persisted in an Azure Storage Table
const Message = require('azure-iot-common').Message;
const iotHubConnectionString = process.env['iotHubConnectionString'];
const cloudClient = require('azure-iothub').Client.fromConnectionString(iotHubConnectionString);
const request = require('request');

function truncateByDot(text, limit){
    return text.length < limit ? text : text.substr(0, limit - 3) + '...';
}

module.exports = function (context, myEventHubMessage) {
    if (!context.bindingData || !context.bindingData.systemProperties || !context.bindingData.systemProperties['iothub-connection-device-id']) {
        context.log('no device id');
        context.done();
        return;
    }
    var deviceId = context.bindingData.systemProperties['iothub-connection-device-id'];
    if (deviceId) {
        cloudClient.open(function (err) {
            if (err) {
                context.log('Could not connect: ' + err.message);
            } else {
                context.log('Client connected');
                let tweet = '';
                let options = {
                    url: process.env['twitterAPI'] + '?count=3&q=%23' + myEventHubMessage.topic || '',
                    headers: {
                        'Authorization': "Bearer " + process.env['twitterBearerKey']
                    }
                };
                request(options, (error, response, body) => {
                    if (!error && response.statusCode == 200) {
                        let info = JSON.parse(body);
                        tweet = (info.statuses && info.statuses.length) ? `@${truncateByDot(info.statuses[0].user.name, 13)}:\n${info.statuses[0].text}` : "No new tweet.";
                        context.log(tweet);
                        const message = new Message(tweet);
                        cloudClient.send(deviceId, message, function (err, res) {
                            if (err) {
                                context.log(`Error in send C2D message: ${err}`);
                            } else {
                                context.log(`send status: ${res.constructor.name}`);
                            }
                            cloudClient.close();
                        });
                    }
                    else {
                        cloudClient.close();
                    }
                });
            }
        });
    }
    context.done();
};