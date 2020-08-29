/* eslint-disable  func-names */
/* eslint quote-props: ["error", "consistent"]*/
/* global AWS*/
/* global putObjectToS3*/
/* global fs*/
/* global deasync*/
/* global cp*/
/* global async*/

// var AWS = require('aws-sdk');
// var fs = require('fs');
// var deasync = require('deasync');
//var cp = require('child_process');
// var async = require('async');


/**
 * This sample demonstrates a simple skill built with the Amazon Alexa Skills
 * nodejs skill development kit.
 * This sample supports multiple lauguages. (en-US, en-GB, de-DE).
 * The Intent Schema, Custom Slots and Sample Utterances for this skill, as well
 * as testing instructions are located at https://github.com/alexa/skill-sample-nodejs-fact
 **/

'use strict';
const Alexa = require('alexa-sdk');
var moment = require('moment');
var now = moment();


//=========================================================================================================================================
//TODO: The items below this comment need your attention.
//=========================================================================================================================================

//Replace with your app ID (OPTIONAL).  You can find this value at the top of your skill's page on http://developer.amazon.com.
//Make sure to enclose your value in quotes, like this: const APP_ID = 'amzn1.ask.skill.-';
const APP_ID = '-';

const SKILL_NAME = 'Feed Chato';
const GET_FEED_MESSAGE = "OK. Chato will be fed ";
const HELP_MESSAGE = 'You can say feed chato now, or, every 3 hours...Let me know.';
const HELP_REPROMPT = 'Should I feed Chato now?';
const STOP_MESSAGE = 'Goodbye!';

//=========================================================================================================================================
//TODO: Replace this data with your own.  You can find translations of this data at http://github.com/alexa/skill-sample-node-js-fact/lambda/data
//=========================================================================================================================================
const data = [
    'it is done.'
];

// 1. Text strings =====================================================================================================
//    Modify these strings and messages to change the behavior of your Lambda function
var myBucket = '-'; //'alexabucket12';      // replace with your own bucket name!
var myObject = '-'; //'hello.txt';          // replace with your own file name!

// var putObjectToS3 = function (bucket, key, data){
//
// };
//=========================================================================================================================================
//Editing anything below this line might break your skill.
//=========================================================================================================================================

exports.handler = function(event, context, callback) {
    var alexa = Alexa.handler(event, context);
    alexa.appId = APP_ID;
    alexa.registerHandlers(handlers);
    alexa.execute();
};

const handlers = {
    'LaunchRequest': function () {
        this.emit('chato');
    },
    'PersonIntent': function () {

    },
    'putObjectToS3': function () {
        console.log("putObjectToS3 handler");
    },
    'feedchato': function () {
        var feedResponseSpeak = "";
        var retText = "";


        var intent = this.event.request.intent.name;
        var durationValue = isSlotValidDuration(this.event.request, "feedduration");
        console.log("feedduration: " + durationValue);
        var duration = moment.duration(durationValue);
        console.log("parsed duration: " + duration); // correct!
        var hourDuration = moment.duration(durationValue).hours();
        var minuteDuration = moment.duration(durationValue).minutes();
        var secondDuration = moment.duration(durationValue).seconds();

        var hourSlotValue = hourDuration;
        var minuteSlotValue = minuteDuration;

        console.log("hourSlotValue, minuteSlotValue, secondDuration: " + hourSlotValue + ", " + minuteSlotValue + ", "+ secondDuration);
        if  (hourSlotValue) {
            //slot has a valid value
            if(hourSlotValue.toString() == "?") {
              console.log("hourSlotValue ? hit!");
              feedResponseSpeak = "Sorry, could not get the number of hours.";
              retText = "NaN";
            } else {
              //not ?
              var hours = parseInt(hourSlotValue);
              console.log("parseinthour: " + hours);
              var seconds = hours * 60 * 60;
              retText = "feedinterval:" + seconds.toString();
              feedResponseSpeak += GET_FEED_MESSAGE + "every " + hours.toString() + " hours";
              console.log("hour feedResponseSpeak: " + feedResponseSpeak);
              // console.log("intent.slots.hour: " + this.event.request.intent.slots.hour.value);
          }
        } else if (minuteSlotValue){
          //possibly ?
          if(minuteSlotValue.toString() == "?") {
            console.log("minute ? hit!");
            feedResponseSpeak = "Sorry, could not get the number of minutes.";
            retText = "NaN";
          } else {
            var minutes = parseInt(minuteSlotValue);
            // var minutes = this.event.request.intent.slots.minute.value;
            var seconds = minutes * 60;
            console.log("parseintminutes: " + seconds);
            retText = "feedinterval:" + seconds.toString();
            feedResponseSpeak += GET_FEED_MESSAGE + "every " + minutes.toString() + " minutes";
            // console.log("minutes.toString(): " + minutes.toString());
            console.log("minute feedResponseSpeak: " + feedResponseSpeak);
            // speechOutput="Intent " + intent + ", did not get a value for " + slotName;
          }

        } else if (secondDuration){
          //possibly ?
          if(secondDuration.toString() == "?") {
            console.log("seconds ? hit!");
            feedResponseSpeak = "Sorry, could not get the number of seconds.";
            retText = "NaN";
          } else {
            // var minutes = parseInt(minuteSlotValue);
            // var minutes = this.event.request.intent.slots.minute.value;
            // var seconds = minutes * 60;
            var seconds = secondDuration;
            console.log("parseintsecs: " + seconds);
            retText = "feedinterval:" + seconds.toString();
            feedResponseSpeak += GET_FEED_MESSAGE + "every " + seconds.toString() + " seconds";
            // console.log("minutes.toString(): " + minutes.toString());
            console.log("seconds feedResponseSpeak: " + feedResponseSpeak);
            // speechOutput="Intent " + intent + ", did not get a value for " + slotName;
          }

        } else {
          //now
          feedResponseSpeak += GET_FEED_MESSAGE + "now";
          retText = "now";
          console.log("now.toString(): ");
        }
        
        console.log("feedResponseSpeak: " + feedResponseSpeak);
        const speechOutput = feedResponseSpeak;

        console.log("before all");

        var myParams = {
            Bucket : myBucket,
            Key : myObject,
            Body : retText
        };
        console.log("myParams.Body: " + myParams.Body.toString());

        S3write(myParams,  myResult => {
          console.log("sent     : " + JSON.stringify(myParams));
          console.log("received : " + JSON.stringify(myResult));

          this.response.cardRenderer(SKILL_NAME, retText);
          this.response.speak(speechOutput);
          this.emit(':responseReady');
        });

        console.log('done');
        console.log("after all");

    },
    'AMAZON.HelpIntent': function () {
        const speechOutput = HELP_MESSAGE;
        const reprompt = HELP_REPROMPT;

        this.response.speak(speechOutput).listen(reprompt);
        this.emit(':responseReady');
    },
    'AMAZON.CancelIntent': function () {
        this.response.speak(STOP_MESSAGE);
        this.emit(':responseReady');
    },
    'AMAZON.StopIntent': function () {
        this.response.speak(STOP_MESSAGE);
        this.emit(':responseReady');
    },
};

// 3. Helper Function  =================================================================================================

function S3write(params, callback) {
    // call AWS S3
    var AWS = require('aws-sdk');
    var s3 = new AWS.S3();

    s3.putObject(params, function(err, data) {
        if(err) { console.log(err, err.stack); }
        else {
            callback(data["ETag"]);

        }
    });
}

function isSlotValid(request, slotName){
    if(request.intent && request.intent.slots) {
      var slot = request.intent.slots[slotName];
      console.log("request = "+JSON.stringify(request)); //uncomment if you want to see the request
      var slotValue;

      //if we have a slot, get the text and store it into speechOutput
      if (slot && slot.value) {
          //we have a value in the slot
          slotValue = slot.value.toLowerCase();
          return slotValue;
      } else {
          //we didn't get a value in the slot.
          return false;
      }
    } else {
      return false;
    }

}
function isSlotValidDuration(request, slotName){
    if(request.intent && request.intent.slots) {
      var slot = request.intent.slots[slotName];
      console.log("duration request = "+JSON.stringify(request)); //uncomment if you want to see the request
      var slotValue;

      //if we have a slot, get the text and store it into speechOutput
      if (slot && slot.value) {
          //we have a value in the slot
          slotValue = slot.value;
          return slotValue;
      } else {
          //we didn't get a value in the slot.
          return false;
      }
    } else {
      return false;
    }

}
