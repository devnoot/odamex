// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// $Id$
//
// Copyright (C) 1993-1996 by id Software, Inc.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// DESCRIPTION:
//  Command-line arguments
//
//-----------------------------------------------------------------------------


#include <algorithm>

#include "cmdlib.h"
#include "m_argv.h"
#include "i_system.h"

IMPLEMENT_CLASS (DArgs, DObject)

DArgs::DArgs ()
{
}

DArgs::DArgs (unsigned int argc, char **argv)
{
	if(argv)
		CopyArgs(argc, argv);
}

DArgs::DArgs (const DArgs &other)
{
	args = other.args;
}

DArgs::DArgs (const char *cmdline)
{
	SetArgs(cmdline);
}

DArgs::~DArgs ()
{
	FlushArgs ();
}

DArgs &DArgs::operator= (const DArgs &other)
{
	args = other.args;
	return *this;
}

const char *DArgs::operator[] (size_t n)
{
	return GetArg(n);
}

void DArgs::SetArgs (unsigned argc, char **argv)
{
	CopyArgs(argc, argv);
}

void DArgs::CopyArgs (unsigned argc, char **argv)
{
	args.clear();

	if(!argv || !argc)
		return;

	args.resize(argc);

	for (unsigned i = 0; i < argc; i++)
		if(argv[i])
			args[i] = argv[i];
}

void DArgs::FlushArgs ()
{
	args.clear();
}

//
// CheckParm
// Checks for the given parameter
// in the program's command line arguments.
// Returns the argument number (1 to argc-1) or 0 if not present
//
size_t DArgs::CheckParm (const char *check) const
{
	if(!check)
		return 0;

	for (size_t i = 1, n = args.size(); i < n; i++)
		if (!stricmp (check, args[i].c_str()))
			return i;

	return 0;
}

const char *DArgs::CheckValue (const char *check) const
{
	if(!check)
		return 0;

	size_t i = CheckParm (check);

	if (i > 0 && i < args.size() - 1)
		return args[i + 1].c_str();
	else
		return NULL;
}

const char *DArgs::GetArg (size_t arg) const
{
	if (arg < args.size())
		return args[arg].c_str();
	else
		return NULL;
}

const std::vector<std::string> DArgs::GetArgList (size_t start) const
{
	std::vector<std::string> out;

	if(start < args.size())
	{
		out.resize(args.size() - start);
		std::copy(args.begin() + start, args.end(), out.begin());
	}

	return out;
}

size_t DArgs::NumArgs () const
{
	return args.size();
}

void DArgs::AppendArg (const char *arg)
{
	if(arg)
		args.push_back(arg);
}

DArgs DArgs::GatherFiles (const char *param, const char *extension, bool acceptNoExt) const
{
	DArgs out;
	size_t i;
	unsigned extlen = strlen (extension);

	for (i = 1; i < args.size() && args[i].length() && args[i][0] != '-' && args[i][0] != '+'; i++)
	{
		if (args[i].length() >= extlen && stricmp (args[i].c_str() + args[i].length() - extlen, extension) == 0)
			out.AppendArg (args[i].c_str());
		else if (acceptNoExt && !strrchr (args[i].c_str(), '.'))
			out.AppendArg (args[i].c_str());
	}
	i = CheckParm (param);
	if (i)
	{
		for (i++; i < args.size() && args[i].length() && args[i][0] != '-' && args[i][0] != '+'; i++)
			out.AppendArg (args[i].c_str());
	}
	return out;
}

void DArgs::SetArgs(const char *cmdline)
{
	char *outputline, *q;
	char **outputargv;
	int outputargc;

	args.clear();

	if(!cmdline)
		return;

	while (*cmdline && isspace(*cmdline))
		cmdline++;

	if (!*cmdline)
		return;

	outputline = (char *)Malloc((strlen(cmdline) + 1) * sizeof(char));
	outputargv = (char **)Malloc(((strlen(cmdline) + 1) / 2) * sizeof(char *));

	const char *p = cmdline;
	q = outputline;
	outputargv[0] = NULL;
	outputargc = 1;

	while (*p)
	{
		int quote;

		while (*p && isspace(*p))
			p++;

		if (!*p)
			break;

		outputargv[outputargc] = q;
		outputargc++;
		quote = 0;

		while (*p)
		{
			if (!quote && isspace(*p))
				break;

			if (*p == '\\' || *p == '\"')
			{
				int slashes = 0, quotes = 0;

				while (*p == '\\')
				{
					slashes++;
					p++;
				}

				while (*p == '"')
				{
					quotes++;
					p++;
				}

				if (!quotes)
					while (slashes--)
						*q++ = '\\';
				else
				{
					while (slashes >= 2)
					{
						slashes -= 2;
						*q++ = '\\';
					}

					if (slashes)
					{
						quotes--;
						*q = '\"';
					}

					if (quotes > 0)
					{
						if (!quote)
						{
							quotes--;
							quote = 1;
						}

						for (int i = 3; i <= quotes + 1; i += 3)
							*q++ = '\"';

						quote = (quotes % 3 == 0);
					}
				}
			}
			else
				*q++ = *p++;
		}

		*q++ = '\0';
	}

	CopyArgs(outputargc, outputargv);

	free(outputargv);
	free(outputline);

	return;
}

#define MAXARGVS 400
//
// Load a Response File
//
//[ ML] 23/1/07 - Add Response file support back in
static void LoadResponseFile (unsigned argv_index)
{
	char		**argv;
	FILE		*handle;
	int 		size;
	int 		k;
	unsigned 	index;
	int 		indexinfile;
	char		*infile;
	char		*file;

	// READ THE RESPONSE FILE INTO MEMORY
	handle = fopen (Args.GetArg(argv_index) + 1,"rb");
	if (!handle)
		I_FatalError ("\nNo such response file!");

	Printf (PRINT_HIGH, "Found response file %s!\n", Args.GetArg(argv_index) + 1);
	fseek (handle, 0, SEEK_END);
	size = ftell (handle);
	fseek (handle, 0, SEEK_SET);
	file = new char[size];
	fread (file, size, 1, handle);
	fclose (handle);

	argv = new char *[size]; // denis - todo - worst case for now (all characters are \n)
	for (index = 0; index < argv_index; index++)
		argv[index] = (char *) Args.GetArg (index);

	infile = file;
	k = 0;
	indexinfile = index;
	do
	{
		argv[indexinfile++] = infile+k;
		while(k < size &&
			  ((*(infile+k)>= ' '+1) && (*(infile+k)<='z')))
			k++;
		*(infile+k) = 0;
		while(k < size &&
			  ((*(infile+k)<= ' ') || (*(infile+k)>'z')))
			k++;
	} while(k < size);

	for (index = argv_index + 1; index < Args.NumArgs (); index++)
		argv[indexinfile++] = (char *) Args.GetArg (index);

	DArgs newargs (indexinfile, argv);
	Args = newargs;

	delete[] file;
	delete[] argv;

#if 0
	// Disabled - Vanilla Doom does not do this.
	// Display arguments

	Printf (PRINT_HIGH, "%d command-line args:\n", Args.NumArgs ());
	for (k = 1; k < Args.NumArgs (); k++)
		Printf (PRINT_HIGH, "%s\n", Args.GetArg (k));
#endif
}

//
// Find a Response File
//
void M_FindResponseFile(void)
{
	unsigned i;

	for (i = 1; i < Args.NumArgs(); i++)
	{
		if (Args.GetArg(i)[0] == '@')
		{
			LoadResponseFile(i);
		}
	}
}


VERSION_CONTROL (m_argv_cpp, "$Id$")

