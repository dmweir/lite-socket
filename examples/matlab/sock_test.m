%% Load the DLL
% Location of lite-socket repo
GIT = 'D:\GIT\';

addpath(fullfile(GIT, 'lite-socket\lib\'))
[notfound,warnings]=loadlibrary('lite_socket',fullfile(GIT, 'lite-socket\include\lite_socket.h'));

% libfunctionsview lite_socket
%% Init winsock
calllib('lite_socket','socket_init')

errbufsz = 512;
errbuf = libpointer('string', blanks(errbufsz));
%% Create a tcp socket
TCP = 1;
UDP = 2;
sock = calllib('lite_socket','socket_create', TCP);

%% Bind to 127.0.0.1:7111 and Display Name
err = calllib('lite_socket','socket_bind', sock, ['127.0.0.1' 0], uint16(7111));

%% Get the socket ip address and port
sn = struct();
psock_name = libstruct('socket_name', sn);
err = calllib('lite_socket','socket_getname', sock, psock_name);
fprintf("%s:%d\n", char(psock_name.ipaddr(psock_name.ipaddr~=0)), psock_name.port);

%% Listen for connections on socket
err = calllib('lite_socket','socket_listen', sock, int32(5));

%% Wait for a connection
[conn, err] = calllib('lite_socket','socket_accept', sock, libpointer('int32Ptr', int32(0)));

%% Read data in a loop from the connection
timeout = 2; %second
while true
    t0 = calllib('lite_socket','get_timestamp');
    status = calllib('lite_socket','socket_read_ready', conn, timeout);
    t1 = calllib('lite_socket','get_timestamp');

    if status < 0
        errmsg = calllib('lite_socket','socket_error_msg', int32(status), errbuf, errbufsz);
        fprintf("%s\n", errmsg)
        break;
    elseif status == 0
        fprintf("Read timeout after %0.6f\n", t1 - t0);
        continue;
    else
        bufsz = int32(20);
        pbuf = libpointer('voidPtr', int8(zeros(1,bufsz)));
        flags = int32(0);
        nbytes = calllib('lite_socket','socket_recv', conn, pbuf, bufsz, flags);
        fprintf("Recv: %s\n", char(pbuf.Value(pbuf.Value ~=0)));
    end
end
%% Send some data
write_timeout = 0; %second

t0 = calllib('lite_socket','get_timestamp');
status = calllib('lite_socket','socket_write_ready', conn, write_timeout);
t1 = calllib('lite_socket','get_timestamp');
fprintf("%0.6fsecs\n", t1-t0);

if status < 0
    errmsg = calllib('lite_socket','socket_error_msg', int32(status), errbuf, errbufsz);
    fprintf("%s\n", errmsg)
elseif status == 0
    fprintf("Write timeout!\n");
else
    payload = int8('Hello Socket');
    pbuf = libpointer('voidPtr', payload);
    flags = int32(0);
    nbytes = calllib('lite_socket','socket_send', conn, pbuf, int32(length(payload)), flags);
    fprintf("Sent: %s\n", char(pbuf.Value(pbuf.Value ~=0)));
end
%% Close the socket
err = calllib('lite_socket','socket_shutdown', sock);
err = calllib('lite_socket','socket_close', sock);

err = calllib('lite_socket','socket_shutdown', conn);
err = calllib('lite_socket','socket_close', conn);

%% Cleanup winsock
calllib('lite_socket','socket_cleanup');
