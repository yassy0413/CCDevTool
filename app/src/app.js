(function(){
"use strict";

var os = require('os');
var fs = require('fs');
var path = require('path');
var express = require('express');
var bodyParser = require('body-parser');
var _ = require('underscore');

var port = 1337;

/**
 * Log window
 */
var logWindow = null;
var checkOutLog = true;
function log(text){
	if( checkOutLog ){
		logWindow.val( logWindow.val() + text + '\n' );
		// Auto scroll
		if( logWindow[0].scrollHeight - (logWindow.scrollTop() + logWindow.height()) < 100 ){
			logWindow.scrollTop( logWindow[0].scrollHeight );
		}
	}
}

/**
 * Server implements
 */
var server = express();
server.use(bodyParser.urlencoded({ extended: true }));
server.use(bodyParser.json());
server.get('/', function (req, res){
	res.write("hello");
	res.end();
});
server.get('/file/exist/:path', function(req, res){
	var targetPath = new Buffer(req.params.path, 'base64').toString();
	fs.stat(window.localStorage.rootPath + targetPath, function(err, data){
		if( err ){
			res.writeHead( 404, {
				'Content-Length': 0,
				'Content-Type': 'application/octet-stream' });
		}else{
			res.writeHead( 200, {
				'Content-Length': 0,
				'Content-Type': 'application/octet-stream' });
		}
		res.end();
		log("E[" + res.statusCode + "]" + targetPath);
	});
});
server.get('/file/data/:path', function(req, res){
	var targetPath = new Buffer(req.params.path, 'base64').toString();
	fs.readFile(window.localStorage.rootPath + targetPath, function(err, data){
		if( err ){
			res.writeHead( 404, {
				'Content-Length': 0,
				'Content-Type': 'application/octet-stream' });
		}else{
			res.writeHead( 200, {
				'Content-Length': data.length,
				'Content-Type': 'application/octet-stream' });
			res.write( data );
		}
		res.end();
		log("D[" + res.statusCode + "]" + targetPath);
	});
});
server.get('/file/list/:path', function(req, res){
	var targetPath = new Buffer(req.params.path, 'base64').toString();
	var rootPath = window.localStorage.rootPath + targetPath;
	try {
		var files = fs.readdirSync(rootPath);
		var ret = "";
		_.each(files, function(file){
			var isDirectory = fs.statSync(rootPath + "/" + file).isDirectory();
			ret += (isDirectory?"1":"0") + file + ",";
		});
		res.writeHead( 200, {
			'Content-Length': ret.length,
			'Content-Type': 'text/plain' });
		res.write( ret );
	}catch(e){
		res.writeHead( 404, {
			'Content-Length': 0,
			'Content-Type': 'text/plain' });
		log(e.message);
	}
	res.end();
	log("L[" + res.statusCode + "]" + targetPath);
});
server.listen(port);

/**
 *
 */
function getLocalIpAddress(){
	var text = "127.0.0.1";
	_.each(os.networkInterfaces(), function(networkInterface){
		var target = _.find(networkInterface, function(iface){
			return iface.family === 'IPv4' && iface.internal === false;
		});
		if( target !== undefined ){
			text = target.address;
		}
	});
	return text;
}

/**
 * onEnter
 */
$(document).ready( function(){
	logWindow = $('#log');

	$('#checkOutLog').click( function(){
	    checkOutLog = this.checked;
	});

	$('#server-info').text( '[' + os.hostname() + '] ' + getLocalIpAddress() + ':' + port );

	$('#reload-button').click( function(){
		chrome.runtime.reload();
	});

	$('#clearlog-button').click( function(){
		logWindow.val( '' );
		logWindow.scrollTop( logWindow[0].scrollHeight );
	});

	// 
	var cancelEvent = function(event){
		event.preventDefault();
		event.stopPropagation();
	};
	$(document).on({
		"dragenter": cancelEvent,
		"dragover": cancelEvent,
		"dragleave": cancelEvent,
		"drop": function(event){
			cancelEvent(event);
		}
	});
	
	// 
	$("#file-select-text").val( window.localStorage.rootPath );
	$('#file-select-text').change( function(){
		window.localStorage.rootPath = $("#file-select-text").val();
	});
	$('#file-select').change( function(){
		window.localStorage.rootPath = path.dirname( $('#file-select').val() ) + "/";
    	$("#file-select-text").val( window.localStorage.rootPath );
	});
});

}());