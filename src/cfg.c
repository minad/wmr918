/***
 *
 * cfg.c - Configuration reader
 *
 * $Id: cfg.c,v 1.1.1.1 2003/11/20 20:08:17 daniel Exp $
 *
 ***/

#include "wmr918.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Prototypes
static Cfg *cfgFind(Cfg *, const char *);

// Read configuration and check for missing variables
void cfgRead(Cfg *cfg, const char *file)
{
    char line[CFG_LINE_SIZE];
    int lineNr;
    FILE *fp;

    fp = fopen(file, "r");
    if (!fp)
    {
        logger(LOG_WARNING, "Configuration \"%s\" couldn't be read: %m\n", file);
        exit(EXIT_FAILURE);
    }

    for (lineNr = 1; fgets(line, sizeof (line), fp); ++lineNr)
    {
        char *p;
        Cfg *newCfg;

        // Ignore comments 
        p = strchr(line, '#');
        if (p != NULL) *p = '\0';

        strtrim(line);
        if (line[0] == '\0')
            continue;
       
        // Ignore wrong lines
        p = strchr(line, '=');
        if (p == NULL)
        {
            logger(LOG_WARNING, "Missing \"=\" in line %d of \"%s\"", lineNr, file);
            continue;
        }

        *p++ = '\0';
        strtrim(line);
        strtrim(p);

        // Find variable
        newCfg = cfgFind(cfg, line);
        if (newCfg == NULL)
        {
            logger(LOG_WARNING, "Unknown variable \"%s=%s\" in line %d of \"%s\"", line, p, lineNr, file);
            continue;
        }    

        // Set variable value
	strncpy(newCfg->value, p, CFG_VALUE_SIZE);
	*(newCfg->value + CFG_VALUE_SIZE - 1) = '\0';
        newCfg->valid = true;
    }

    fclose(fp);

    // Check all variables
    while (cfg->name)
    {
        if (!cfg->valid)
        {
            logger(LOG_ERR, "Variable \"%s\" not found in \"%s\"", cfg->name, file);
            exit(EXIT_FAILURE);
        }
        ++cfg;
    }
}

// Get value of configuration variable
const char *cfgValue(Cfg *cfg, const char *name)
{
    return (cfgFind(cfg, name)->value);
}

// Find configuration variable
static Cfg *cfgFind(Cfg *cfg, const char *name)
{
    while (cfg->name)
    {
        if (!strcmp(cfg->name, name))
            return cfg;
	++cfg;
    }
    return NULL;
}
