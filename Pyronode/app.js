
io.sockets.on('connection',function(socket){ 
	
		var clients = io.sockets.clients();
		var total = clients.length;
	
    io.sockets.sockets['nickname'] = socket.id;
		io.sockets.sockets['key'] = Math.round(Math.random*Math.random/Math.random)*9999999;
		
		if(total == 1) socket.set('sessionController', socket.id, function(){
			
		})
		
		socket.emit('session.start', {
			
		})
		
    client.on("pipe", function(data) {      
	
        var sock_id = io.sockets.sockets['nickname'];
				var active_id = socket.get('sessionController'); //This is the ID that has control over the Sphere.
				if(active_id != sock_id) io.sockets.sockets[sock_id].emit("session", { key  : false } );
				// else serial.send(data.req);
				// else io.sockets.sockets[sock_id].emit("private", "message");     

    });

});