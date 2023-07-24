myid = 99999;
function set_uid(x)
	myid = x;
end

function event_player_move(player)
	player_x = API_get_x_p(player);
	player_y = API_get_y_p(player);
	my_x = API_get_x_n(myid);
	my_y = API_get_y_n(myid);
	if (player_x == my_x) then
		if (player_y == my_y) then
		API_SendMessage(myid, player, "HELLO");
		end
	end
end

