/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 *   Copyright (C) 2017 Carlos Ferry <carlos.ferry@gmail.com>
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

/* Handle /BIGCYCLE - Parts and Join all users of a determined channel. */
 
class CommandBigCycle : public SplitCommand
{
 public:

	CommandBigCycle(Module* Creator)
		: SplitCommand(Creator, "BIGCYCLE", 1)
	{
		Penalty = 3; 
		syntax = "<channel> :[reason]";
	}

	CmdResult HandleLocal(const std::vector<std::string> &parameters, LocalUser* user)
	{
		if ((user) && (user->registered == REG_ALL))
                {
                        if (!user->HasPrivPermission("users/sajoin-others", false))
                        {
                                user->WriteNotice("*** You are not allowed to /BIGCYCLE channels  (the privilege users/sajoin-others is needed).");
                                return CMD_FAILURE;
                        }

                        if (user->server->IsULine())
                        {
                                user->WriteNumeric(ERR_NOPRIVILEGES, "Cannot use an SA command on a u-lined client");
                                return CMD_FAILURE;
                        }
		
		}
		
		
		Channel* channel = ServerInstance->FindChan(parameters[0]);
		
		std::string reason = "BigCycle";

		if (parameters.size() > 1)
		{
			/* reason provided, use it */
			
			reason = reason + ": " + parameters[1];
		}

		if (!channel)
		{
			user->WriteNumeric(ERR_NOSUCHCHANNEL, parameters[0], "No such channel");
			return CMD_FAILURE;
		}

		const Channel::MemberMap& cu = channel->GetUsers();
               
		for (Channel::MemberMap::const_iterator i = cu.begin(); i != cu.end(); ++i)
		{
			
			LocalUser* localuser = IS_LOCAL(i->second->user);
		
			/* Oper running this command won't be cycled. */
		
			if (user != i->first)
			{
				user->WriteNumeric(ERR_NOSUCHCHANNEL, i->second->user->nick, ": Parting User ");
				channel->PartUser(i->second->user, reason);
				user->WriteNumeric(ERR_NOSUCHCHANNEL, localuser->nick, ": Joining User ");
				Channel::JoinUser(localuser, parameters[0], true);
			}

		}
		
	       return CMD_SUCCESS;
	}
};


class ModuleBigCycle : public Module
{
 
 private:
 
	CommandBigCycle cmd;

 public:
	ModuleBigCycle() : cmd(this)
	{
	
	}

	Version GetVersion() CXX11_OVERRIDE
	{
		return Version("Provides command BIGCYCLE, parts and joins all users of a determined channel.", VF_VENDOR);
	}
};

MODULE_INIT(ModuleBigCycle)
