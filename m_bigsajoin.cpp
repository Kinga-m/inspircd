/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 *   Copyright (C) 2017 Carlos F. Ferry <carlos.ferry@gmail.com>
 *
 * This file is part of InspIRCd.  InspIRCd is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "inspircd.h"

/* Handle /BIGSAJOIN - Joins all users on server to a determind channel */

class CommandBigSajoin : public Command
{
  
  public:
	
	CommandBigSajoin(Module* Creator) : Command(Creator,"BIGSAJOIN", 1)
	{
		allow_empty_last_param = false;
		flags_needed = 'o'; Penalty = 0; syntax = "[<nick>] <channel>[,<channel>]";
		TRANSLATE2(TR_NICK, TR_TEXT);
	}

	CmdResult Handle (const std::vector<std::string>& parameters, User *user)
	{
		const unsigned int channelindex = (parameters.size() > 1) ? 1 : 0;
	
		if (CommandParser::LoopCall(user, this, parameters, channelindex))
		{
			return CMD_FAILURE;
		}

		const std::string& channel = parameters[channelindex];
		const std::string& nickname = parameters.size() > 1 ? parameters[0] : user->nick;

		User* dest = ServerInstance->FindNick(nickname);
		
		if ((dest) && (dest->registered == REG_ALL))
		{
			
			/* Verify whether the oper has permission to run this command */
			
			if (!user->HasPrivPermission("users/sajoin-others", false))
			{
				user->WriteNotice("*** You are not allowed to use /BIGSAJOIN  (the privilege users/sajoin-others is needed).");
				return CMD_FAILURE;
			}

			if (dest->server->IsULine())
			{
				user->WriteNumeric(ERR_NOPRIVILEGES, "Cannot use an SA command on a u-lined client");
				return CMD_FAILURE;
			}
		
			if (IS_LOCAL(user) && !ServerInstance->IsChannel(channel))
			{
				user->WriteNotice("*** Invalid characters in channel name or name too long");
				return CMD_FAILURE;
			}
		}
		
		Channel* chan = ServerInstance->FindChan(channel);
			
		if (!chan)
		{
			user->WriteRemoteNotice("*** Unable to bigsajoin: Channel does not exist: " + channel);
		        return CMD_FAILURE;
		}

			
		/* Retrieves all users as users' hash */
			
		const user_hash& users = ServerInstance->Users->GetUsers();

                for (user_hash::const_iterator i = users.begin(); i != users.end(); ++i)
                {
			LocalUser* localuser = IS_LOCAL(i->second);
                        
                        if (localuser)
                        {
                                chan = Channel::JoinUser(localuser, channel, true);
                        
                                if (chan)
                                {
                                        ServerInstance->SNO->WriteGlobalSno('a', user->nick + " used SAJOIN to make "+dest->nick+" join "+channel);
                                }
                                else
                                {
                                        user->WriteNotice("*** Could not join " + dest->nick + " to " + channel);
                                }
			}

		}		
		
		return CMD_SUCCESS;
	}

	RouteDescriptor GetRouting(User* user, const std::vector<std::string>& parameters)
	{
		return ROUTE_OPT_UCAST(parameters[0]);
	}
};

class ModuleBigSajoin : public Module
{
	
 private:
 	
	CommandBigSajoin cmd;
 public:
 
	ModuleBigSajoin() : cmd(this)
	{
	
	}

	Version GetVersion() CXX11_OVERRIDE
	{
		return Version("Provides command BIGSAJOIN to allow opers to mass-force-join users to channels.", VF_OPTCOMMON | VF_VENDOR);
	}
};

MODULE_INIT(ModuleBigSajoin)
