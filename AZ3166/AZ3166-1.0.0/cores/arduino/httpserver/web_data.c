﻿/**
 ******************************************************************************
 *
 *  The MIT License
 *  Copyright (c) 2016 MXCHIP Inc.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is furnished
 *  to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 *  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
 *  IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 ******************************************************************************
 */

const char * page_head = "<!DOCTYPE html><html lang=\"en\"><head> <meta charset=\"UTF-8\"> <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> <meta http-equiv=\"X-UA-Compatible\" content=\"ie=edge\"> <title>AZ3166 WiFi Config</title> <style>@charset \"UTF-8\";/*Flavor name: Default (mini-default)Author: Angelos Chalaris (chalarangelo@gmail.com)Maintainers: Angelos Chalarismini.css version: v2.1.5 (Fermion)*//*Browsers resets and base typography.*/html{font-size: 16px;}html, *{font-family: -apple-system, BlinkMacSystemFont,\"Segoe UI\",\"Roboto\", \"Droid Sans\",\"Helvetica Neue\", Helvetica, Arial, sans-serif; line-height: 1.5; -webkit-text-size-adjust: 100%;}*{font-size: 1rem;}body{margin: 0; color: #212121; background: #f8f8f8;}section{display: block;}input{overflow: visible;}h1, h2{line-height: 1.2em; margin: 0.75rem 0.5rem; font-weight: 500;}h2 small{color: #424242; display: block; margin-top: -0.25rem;}h1{font-size: 2rem;}h2{font-size: 1.6875rem;}p{margin: 0.5rem;}small{font-size: 0.75em;}a{color: #0277bd; text-decoration: underline; opacity: 1; transition: opacity 0.3s;}a:visited{color: #01579b;}a:hover, a:focus{opacity: 0.75;}/*Definitions for the grid system.*/.container{margin: 0 auto; padding: 0 0.75rem;}.row{box-sizing: border-box; display: -webkit-box; -webkit-box-flex: 0; -webkit-box-orient: horizontal; -webkit-box-direction: normal; display: -webkit-flex; display: flex; -webkit-flex: 0 1 auto; flex: 0 1 auto; -webkit-flex-flow: row wrap; flex-flow: row wrap;}[class^='col-sm-']{box-sizing: border-box; -webkit-box-flex: 0; -webkit-flex: 0 0 auto; flex: 0 0 auto; padding: 0 0.25rem;}.col-sm-10{max-width: 83.33333%; -webkit-flex-basis: 83.33333%; flex-basis: 83.33333%;}.col-sm-offset-1{margin-left: 8.33333%;}@media screen and (min-width: 768px){.col-md-4{max-width: 33.33333%; -webkit-flex-basis: 33.33333%; flex-basis: 33.33333%;}.col-md-offset-4{margin-left: 33.33333%;}}/*Definitions for navigation elements.*/header{display: block; height: 2.75rem; background: #1e6bb8; color: #f5f5f5; padding: 0.125rem 0.5rem; white-space: nowrap; overflow-x: auto; overflow-y: hidden;}header .logo{color: #f5f5f5; font-size: 1.35rem; line-height: 1.8125em; margin: 0.0625rem 0.375rem 0.0625rem 0.0625rem; transition: opacity 0.3s;}header .logo{text-decoration: none;}/*Definitions for forms and input elements.*/form{background: #eeeeee; border: 1px solid #c9c9c9; margin: 0.5rem; padding: 0.75rem 0.5rem 1.125rem;}.input-group{display: inline-block;}.input-group.fluid{display: -webkit-box; -webkit-box-pack: justify; display: -webkit-flex; display: flex; -webkit-align-items: center; align-items: center; -webkit-justify-content: center; justify-content: center;}.input-group.fluid > input{-webkit-box-flex: 1; max-width: 100%; -webkit-flex-grow: 1; flex-grow: 1; -webkit-flex-basis: 0; flex-basis: 0;}@media screen and (max-width: 767px){.input-group.fluid{-webkit-box-orient: vertical; -webkit-align-items: stretch; align-items: stretch; -webkit-flex-direction: column; flex-direction: column;}}[type=\"password\"], select{box-sizing: border-box; background: #fafafa; color: #212121; border: 1px solid #c9c9c9; border-radius: 2px; margin: 0.25rem; padding: 0.5rem 0.75rem;}input:not([type=\"button\"]):not([type=\"submit\"]):not([type=\"reset\"]):hover, input:not([type=\"button\"]):not([type=\"submit\"]):not([type=\"reset\"]):focus, select:hover, select:focus{border-color: #0288d1; box-shadow: none;}input:not([type=\"button\"]):not([type=\"submit\"]):not([type=\"reset\"]):disabled, select:disabled{cursor: not-allowed; opacity: 0.75;}::-webkit-input-placeholder{opacity: 1; color: #616161;}::-moz-placeholder{opacity: 1; color: #616161;}::-ms-placeholder{opacity: 1; color: #616161;}::placeholder{opacity: 1; color: #616161;}button::-moz-focus-inner, [type=\"submit\"]::-moz-focus-inner{border-style: none; padding: 0;}button, [type=\"submit\"]{-webkit-appearance: button;}button{overflow: visible; text-transform: none;}button, [type=\"submit\"], a.button, .button{display: inline-block; background: rgba(208, 208, 208, 0.75); color: #212121; border: 0; border-radius: 2px; padding: 0.5rem 0.75rem; margin: 0.5rem; text-decoration: none; transition: background 0.3s; cursor: pointer;}button:hover, button:focus, [type=\"submit\"]:hover, [type=\"submit\"]:focus, a.button:hover, a.button:focus, .button:hover, .button:focus{background: #d0d0d0; opacity: 1;}button:disabled, [type=\"submit\"]:disabled, a.button:disabled, .button:disabled{cursor: not-allowed; opacity: 0.75;}/*Custom elements for forms and input elements.*/button.primary, [type=\"submit\"].primary, .button.primary{background: rgba(30, 107, 184, 0.9); color: #fafafa;}button.primary:hover, button.primary:focus, [type=\"submit\"].primary:hover, [type=\"submit\"].primary:focus, .button.primary:hover, .button.primary:focus{background: #0277bd;}#content{margin-top: 2em;}</style></head>";
const char * wifi_setting_a = "<body> <header> <h1 class=\"logo\">AZ3166 WiFi Config</h1> </header> <section class=\"container\"> <div id=\"content\" class=\"row\"> <div class=\"col-sm-10 col-sm-offset-1 col-md-4 col-md-offset-4\" style=\"text-align:center;\"> <form action=\"result\" method=\"post\" enctype=\"multipart/form-data\"> <div class=\"input-group fluid\"> <select name=\"SSID\" id=\"SSID\" style=\"width:100%;\"> ";
const char * wifi_setting_b = "</select> </div><div class=\"input-group fluid\"> <input type=\"password\" value=\"\" name=\"PASS\" id=\"password\" placeholder=\"Password\" style=\"width:100%;\"> </div><div class=\"input-group fluid\"> <button type=\"submit\" class=\"primary\">Connect</button> </div></form> <h5>Please refresh this page to update SSID if you cannot find it from the list</h5> </div></div></section></body></html>";
const char * success_result = "%s<body> <header> <h1 class=\"logo\">AZ3166 WiFi Config</h1> </header> <section class=\"container\"> <div id=\"content\" class=\"row\"> <div class=\"col-sm-10 col-sm-offset-1 col-md-4 col-md-offset-4\" style=\"text-align:center;\"> <h2 style=\"color:#2e7d32;\">Connecting to WiFi scceeded.<small>SSID: %s, IP Address: %s</small></h2> <h5>Wait a few seconds for the DevKit to reboot...</h5> </div></div></section></body></html>";
const char * failed_result = "%s<body> <header> <h1 class=\"logo\">AZ3166 WiFi Config</h1> </header> <section class=\"container\"> <div id=\"content\" class=\"row\"> <div class=\"col-sm-10 col-sm-offset-1 col-md-4 col-md-offset-4\" style=\"text-align:center;\"> <h2 style=\"color:#c62828;\">Connecting to WiFi failed.<small>SSID: %s</small></h2> <p><a href=\"/\" class=\"button primary\" target=\"_self\">Retry</a></p></div></div></section></body></html>";