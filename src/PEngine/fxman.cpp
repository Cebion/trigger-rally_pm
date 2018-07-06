
// fxman.cpp [pengine]

// Copyright 2004-2006 Jasmine Langridge, jas@jareiko.net
// License: GPL version 2 (see included gpl.txt)


#include "pengine.h"
#include "physfs_utils.h"

// @todo: what these two global things are?
fx_renderstate_s def_rs;

int max_tex_units;



PSSEffect::PSSEffect(PApp &parentApp) : PSubsystem(parentApp)
{
  PUtil::outLog() << "Initialising effects subsystem" << std::endl;

  def_rs.depthtest = true;

  def_rs.lighting = true;
  def_rs.lightmodeltwoside = false;

  def_rs.alphatest.func = GL_ALWAYS;
  def_rs.alphatest.ref = 0.0; // not important for GL_ALWAYS anyway

  def_rs.cullface = CULLFACE_CW;

  def_rs.blendmode = BLEND_NONE;

  def_rs.texunit[0].texindex = -1;

  glGetIntegerv(GL_MAX_TEXTURE_UNITS, &max_tex_units);
}

PSSEffect::~PSSEffect()
{
  PUtil::outLog() << "Shutting down effects subsystem" << std::endl;

  fxlist.clear();
}


PEffect *PSSEffect::loadEffect(const std::string &name)
{
  PEffect *fx = fxlist.find(name);
  if (!fx) {
    try
    {
      fx = new PEffect (name);
    }
    catch (PException &e)
    {
      if (PUtil::isDebugLevel(DEBUGLEVEL_ENDUSER))
        PUtil::outLog() << "Failed to load " << name << ": " << e.what () << std::endl;
      return nullptr;
    }
    fx->setFirstValidTechnique();
    fxlist.add(fx);
  }
  return fx;
}

// ???
#define con_printf(...)




PEffect::~PEffect()
{
  unload ();
}

void PEffect::unload()
{
  tex.clear ();
  tech.clear ();
}

PEffect::PEffect(const std::string &filename)
{
   cur_tech = -1;
   
   /* Let's check each effect type will load (FX or OBJ's MTL) */
   if(filename.find(".fx") != std::string::npos)
   {
      loadFX(filename);
   }
   else
   {
      loadMTL(filename);
   }
}

/*! Load an OBJ's MTL file as the effect definition.
 * \note: It just create a default PEffect using the material first texture,
 *        ignoring all material real things (unused by the renderer, btw) */
#define BUF_SIZE 1024
void PEffect::loadMTL(const std::string &filename)
{
   char buff[BUF_SIZE];             /**< File buffer */
   PHYSFS_file* pfile;          /**< The real .obj file */
   std::string tok;             /**< Readed token from line */
   std::string value;           /**< Readed value from line */
   int numMaterials=0;          /**< Number of materials readed */

   if(PUtil::isDebugLevel(DEBUGLEVEL_TEST))
   {
      PUtil::outLog() << "Loading .mtl Material \"" << filename 
         << "\"" << std::endl;
   }

   /* Unload previous loaded effect */
   unload();

   /* Let's read from file */
   pfile = PHYSFS_openRead(filename.c_str());
   if(pfile == nullptr)
   {
      con_printf("Cannot find effect file \"%s\"\n",filename);
      throw MakePException ("PhysFS: " + physfs_getErrorString());
   }

   name = filename;
   
   /* Loop throught all file */
   while(PUtil::fgets2(buff,BUF_SIZE,pfile))
   {
      if(PUtil::getToken(buff, tok, value))
      {
         if(numMaterials > 1)
         {
            /* FIXME: ignoring all further materials. */
         }
         else if(tok == "newmtl")
         {
            numMaterials++;
            /* FIXME: ignoring material name, as it's already the filename. */
         }
         else if(tok == "map_Kd")
         {
            /* Define the FX single texture */
            tex.push_back(fx_texture_s());
            fx_texture_s *curtex = &tex.back();
            /* Already as 2D texture. */
            curtex->texobject = NULL;
            curtex->name = value;
            curtex->type = GL_TEXTURE_2D;
            curtex->filename = PUtil::assemblePath(value, filename);

            /* Define the default Technique */
            tech.push_back(fx_technique_s());
            fx_technique_s *curtech = &tech.back();
            curtech->name = "DefaultTechMTL";
            /* And its default pass and render state */
            curtech->pass.push_back(fx_pass_s());
            fx_pass_s *curpass = &curtech->pass.back();
            fx_renderstate_s *currs = &curpass->rs;
            /* Uhm? Copyed from the loadFX bellow... Just set some defaults,
             * and the the texture to the only one defined. */
            *currs = def_rs;
            currs->depthtest = true;
            currs->cullface = CULLFACE_CW;
            currs->texunit[0].texindex = 0;
         }
         else
         {
            /* Ignore everithing, as the FX will only use the texture. */
         }
      }
   }

   /* Done! */
   PHYSFS_close(pfile);
}
#undef BUF_SIZE

void PEffect::loadFX(const std::string &filename)
{
	if (PUtil::isDebugLevel(DEBUGLEVEL_TEST))
		PUtil::outLog() << "Loading fx shader \"" << filename << "\"" << std::endl;

	// unload eventual previous loaded effect
	unload();

	// open file
	PHYSFS_file *pfile = PHYSFS_openRead(filename.c_str());
	if (pfile == nullptr)
	{
		con_printf("Cannot find effect file \"%s\"\n",filename);
		throw MakePException ("PhysFS: " + physfs_getErrorString());
	}
	
	// set the name
	name = filename;

	// we'll create a string storing the whole file
	int leng = PHYSFS_fileLength(pfile);
	char *source = new char[leng+1];
	physfs_read(pfile, source, sizeof(char), leng);
	source[leng] = '\0';
	
	// close file since we have everything in char *source
	PHYSFS_close(pfile);

#define SKIPWHITESPACE \
{ \
	while (*scan == ' ' || *scan == '\t' || *scan == '\n' || *scan == '\r') { \
		if (*scan == '\n') linec++; \
		scan++; \
	} \
}

#define READTOKEN \
{ \
	token = buff1; \
	if (*scan == '\"') { \
		scan++; \
		while ((*scan) && (*scan != '\"')) { \
			*(token++) = *(scan++); \
		} *token = '\0'; token = buff1; scan++; \
	} else { \
		while ((*scan >= 'a' && *scan <= 'z') || \
			(*scan >= 'A' && *scan <= 'Z') || \
			(*scan >= '0' && *scan <= '9') || \
			(*scan == '_')) { \
			*(token++) = *(scan++); \
		} *token = '\0'; token = buff1; \
	} \
}

#define READNUMERICALTOKEN \
{ \
	token = buff1; \
	while ((*scan >= '0' && *scan <= '9') || \
	(*scan == '-' || *scan == '.')) \
	{ \
		*(token++) = *(scan++); \
	} \
	*token = '\0'; \
	token = buff1; \
}

#define FAILURE \
{ \
	parseerror = true; \
	break; \
}

	char buff1[512];
  
	char *token;

	// In case of failures, stores short descriptive string
	std::string detail("parse error");
  
	int linec = 1;
	bool parseerror = false;

	// this is the pointer to the char in source that's to be read next
	char* scan = source;
	
	while (1)
	{
		SKIPWHITESPACE;
		if (!*scan)
			break;

		READTOKEN;
		if (!*scan)
			FAILURE;

		if (!strcmp(token, "texture"))
		{
			tex.push_back(fx_texture_s());
			fx_texture_s *curtex = &tex.back();
      
			curtex->texobject = nullptr;

			SKIPWHITESPACE;
			if (!*scan)
				FAILURE;

			READTOKEN;
			if (!*scan)
				FAILURE;
			
			curtex->name = token;

			SKIPWHITESPACE;
			if (!*scan)
			{
				parseerror = true;
				break;
			}
			if (*scan != '{')
				FAILURE;
			
			scan++;

			while (1)
			{
				SKIPWHITESPACE;
				if (!*scan)
				FAILURE;
			
				if (*scan == '}')
				{
					scan++;
					break;
				}

				READTOKEN;
				if (!*scan)
					FAILURE;
			

				// type = [ TEX_2D | TEX_3D | TEX_CUBE ];
				if (!strcmp(token, "type"))
				{
					SKIPWHITESPACE;
					if (!*scan)
						FAILURE;
			
					if (*scan != '=')
						FAILURE;
			
					scan++;
					SKIPWHITESPACE;
					if (!*scan)
						FAILURE;
			
					READTOKEN;
					if (!*scan)
						FAILURE;
			
					if (!strcmp(token,"TEX_2D"))
						curtex->type = GL_TEXTURE_2D;
					else if (!strcmp(token,"TEX_3D"))
						curtex->type = GL_TEXTURE_3D;
					else if (!strcmp(token,"TEX_CUBE"))
						curtex->type = GL_TEXTURE_CUBE_MAP;
					else
					{
						parseerror = true;
						break;
					}
					SKIPWHITESPACE;
					if (!*scan)
						FAILURE;
			
					if (*scan != ';')
						FAILURE;
			
					scan++;
				}
				// src = "blah.png";
				else if (!strcmp(token, "src"))
				{
					SKIPWHITESPACE;
					if (!*scan)
						FAILURE;
			
					if (*scan != '=')
						FAILURE;
			
					scan++;
					SKIPWHITESPACE;
					if (!*scan)
						FAILURE;
			
					READTOKEN;
					if (!*scan)
						FAILURE;
			
					curtex->filename = PUtil::assemblePath(token, filename);
					SKIPWHITESPACE;
					if (!*scan)
						FAILURE;
			
					if (*scan != ';')
						FAILURE;
			
					scan++;
				}
				else
					FAILURE;
			}
		}
		else if (!strcmp(token, "technique"))
		{
			tech.push_back(fx_technique_s());
			fx_technique_s *curtech = &tech.back();
      
			SKIPWHITESPACE;
			if (!*scan)
				FAILURE;
			

			READTOKEN;
			if (!*scan)
				FAILURE;
			
			curtech->name = token;

			SKIPWHITESPACE;
			if (!*scan)
				FAILURE;
			
			if (*scan != '{')
				FAILURE;
			
			scan++;

			while (1)
			{
				SKIPWHITESPACE;
				if (!*scan)
					FAILURE;
			
				if (*scan == '}')
					FAILURE;
			

				READTOKEN;
				if (!*scan)
					FAILURE;
			
        
				if (!strcmp(token, "pass"))
				{
					curtech->pass.push_back(fx_pass_s());
					fx_pass_s *curpass = &curtech->pass.back();
					fx_renderstate_s *currs = &curpass->rs;
					// set default render state for pass
					#if 0
					if (curtech->pass.size() > 1)
						*currs = curtech->pass[curtech->pass.size()-2].rs;
					else
						*currs = def_rs;
					#else
						*currs = def_rs;
					#endif

					SKIPWHITESPACE;
					if (!*scan)
						FAILURE;
			
					if (*scan != '{')
						FAILURE;
			
					scan++;

					while (1)
					{
						SKIPWHITESPACE;
						if (!*scan)
							FAILURE;
			
						if (*scan == '}')
							FAILURE;
			

						READTOKEN;
						if (!*scan)
							FAILURE;
			

						// depthtest = [ true | false ];
						if (!strcmp(token, "depthtest"))
						{
							//con_printf("depthtest\n");
							SKIPWHITESPACE;
							if (!*scan)
								FAILURE;
			
							if (*scan != '=')
								FAILURE;
			
							scan++;
							SKIPWHITESPACE;
							if (!*scan)
								FAILURE;
			
							READTOKEN;
							if (!*scan)
								FAILURE;
			
							if (0);
							else if (!strcmp(token,"true"))
								currs->depthtest = true;
							else if (!strcmp(token,"false"))
								currs->depthtest = false;
							else
								FAILURE;
			
							SKIPWHITESPACE;
							if (!*scan)
								FAILURE;
			
							if (*scan != ';')
								FAILURE;
			
							scan++;
						}
						// alphatest = { FUNC, value };
						else if (!strcmp(token, "alphatest"))
						{
							//con_printf("alphatest\n");
							SKIPWHITESPACE;
							if (!*scan)
								FAILURE;
			
							if (*scan != '=')
								FAILURE;
			
							scan++;
							SKIPWHITESPACE;
							if (!*scan)
								FAILURE
			
							if (*scan != '{')
								FAILURE
			
							scan++;
							SKIPWHITESPACE;
							if (!*scan)
								FAILURE
			
							READTOKEN;
							if (!*scan)
								FAILURE
			
							if (0);
							else if (!strcmp(token,"LESS"))
								currs->alphatest.func = GL_LESS;
							else if (!strcmp(token,"EQUAL"))
								currs->alphatest.func = GL_EQUAL;
							else if (!strcmp(token,"LEQUAL"))
								currs->alphatest.func = GL_LEQUAL;
							else if (!strcmp(token,"GREATER"))
								currs->alphatest.func = GL_GREATER;
							else if (!strcmp(token,"NOTEQUAL"))
								currs->alphatest.func = GL_NOTEQUAL;
							else if (!strcmp(token,"GEQUAL"))
								currs->alphatest.func = GL_GEQUAL;
							else if (!strcmp(token,"ALWAYS"))
								currs->alphatest.func = GL_ALWAYS;
							else
							{
								detail = "invalid alphatest func";
								FAILURE
							}
							SKIPWHITESPACE;
							if (!*scan)
								FAILURE;
			
							if (*scan != ',')
								FAILURE;
			
							scan++;
							SKIPWHITESPACE;
							if (!*scan)
								FAILURE;
			
							READNUMERICALTOKEN;
							if (!*scan)
								FAILURE;
			
							currs->alphatest.ref = atof(token);
							SKIPWHITESPACE;
							if (!*scan)
								FAILURE;
			
							if (*scan != '}')
								FAILURE;
				
							scan++;
							SKIPWHITESPACE;
							if (!*scan)
								FAILURE;
			
							if (*scan != ';')
								FAILURE;
			
							scan++;
						}
						// cullface
						else if (!strcmp(token, "cullface"))
						{
							//con_printf("cullface\n");
							SKIPWHITESPACE;
							if (!*scan)
								FAILURE;
			
							if (*scan != '=')
								FAILURE;
			
							scan++;
							SKIPWHITESPACE;
							if (!*scan)
								FAILURE;
			
							READTOKEN;
							if (!*scan)
								FAILURE;
			
							if (0);
							else if (!strcmp(token,"NONE"))
								currs->cullface = CULLFACE_NONE;
							else if (!strcmp(token,"CW"))
								currs->cullface = CULLFACE_CW;
							else if (!strcmp(token,"CCW"))
								currs->cullface = CULLFACE_CCW;
							else
							{
								detail = "invalid cullface mode";
								FAILURE
							}
							SKIPWHITESPACE;
							if (!*scan)
								FAILURE;
			
							if (*scan != ';')
								FAILURE;
			
							scan++;
						}
						// blendmode
						else if (!strcmp(token, "blendmode"))
						{
							SKIPWHITESPACE;
							if (!*scan)
								FAILURE;
			
							if (*scan != '=')
								FAILURE;
			
							scan++;
							SKIPWHITESPACE;
							if (!*scan)
								FAILURE;

							READTOKEN;
							if (!*scan)
								FAILURE;
			
							if (0);
							else if (!strcmp(token,"NONE"))
								currs->blendmode = BLEND_NONE;
							else if (!strcmp(token,"ADD"))
								currs->blendmode = BLEND_ADD;
							else if (!strcmp(token,"MULTIPLY"))
								currs->blendmode = BLEND_MULTIPLY;
							else if (!strcmp(token,"ALPHA"))
								currs->blendmode = BLEND_ALPHA;
							else if (!strcmp(token,"PREMULTALPHA"))
								currs->blendmode = BLEND_PREMULTALPHA;
							else
							{
								detail = "invalid blendmode mode";
								FAILURE
							}
							SKIPWHITESPACE;
							if (!*scan)
								FAILURE;
			
							if (*scan != ';')
								FAILURE;
			
							scan++;
						}
						// texunit0 = <texture>;
						else if (!strcmp(token, "texunit0"))
						{
							//con_printf("texunit0\n");
							SKIPWHITESPACE;
							if (!*scan)
								FAILURE;
			
							if (*scan != '=')
								FAILURE;
			
							scan++;
							SKIPWHITESPACE;
							if (!*scan)
								FAILURE;
			
							READTOKEN;
							if (!*scan)
								FAILURE;
			
							unsigned int iter;
							for (iter=0; iter<tex.size(); iter++)
							{
								if (!strcmp(token, tex[iter].name.c_str()))
									break;
							}
							if (iter == tex.size())
							{
								detail = std::string("texture name ") + token + " not found";
								parseerror = true;
								break;
							}
							currs->texunit[0].texindex = iter;
							SKIPWHITESPACE;
							if (!*scan)
								FAILURE;
			
							if (*scan != ';')
								FAILURE;
			
							scan++;
						}
						else if (!strcmp(token, "lighting"))
						{
							SKIPWHITESPACE;
							if (!*scan)
								FAILURE;
			
							if (*scan != '=')
								FAILURE;
			
							scan++;
							SKIPWHITESPACE;
							if (!*scan)
								FAILURE;
			
							READTOKEN;
							if (!*scan)
								FAILURE;
			
							if (0);
							else if (!strcmp(token,"true"))
								currs->lighting = true;
							else if (!strcmp(token,"false"))
								currs->lighting = false;
							else
								FAILURE;
			
							SKIPWHITESPACE;
							if (!*scan)
								FAILURE;
			
							if (*scan != ';')
								FAILURE;
			
							scan++;
						}
						else if (!strcmp(token, "lightmodeltwoside"))
						{
							SKIPWHITESPACE;
							if (!*scan)
								FAILURE;
			
							if (*scan != '=')
								FAILURE;
			
							scan++;
							SKIPWHITESPACE;
							if (!*scan)
								FAILURE;
			
							READTOKEN;
							if (!*scan)
								FAILURE;
			
							if (0);
							else if (!strcmp(token,"true"))
								currs->lightmodeltwoside = true;
							else if (!strcmp(token,"false"))
								currs->lightmodeltwoside = false;
							else
								FAILURE;
			
							SKIPWHITESPACE;
							if (!*scan)
								FAILURE;
			
							if (*scan != ';')
								FAILURE;
			
							scan++;
						}
						else
						{
							detail = "invalid render state";
							FAILURE
						}
					}
				}
				else
					FAILURE;
			
				if (parseerror)
					break;
			}
		}
		else
			FAILURE;
		
		if (parseerror)
			break;
	}

	delete [] source;

	if (parseerror)
	{
		con_printf("\"%s\": error at line %i : %s\n",name.c_str(),linec,detail);
		//*scan = '\0';
		//con_printf("[%s]\n",source);
		unload();
		throw MakePException (name + ": error at line " + PUtil::formatInt (linec) + " : " + detail);
	}
  
	con_printf("Load complete\n");
}

int PEffect::getNumTechniques()
{
  return tech.size();
}

bool PEffect::validateTechnique(int technique)
{
  // todo: determine real validity

  // even for fixed-function, this includes (eg) tex unit count

  if (technique >= 0 && technique < (int)tech.size()) {
    tech[technique].validated = true;
    return tech[technique].validated;
  } else {
    return false;
  }
}

const std::string &PEffect::getTechniqueName(int technique)
{
  static std::string notfound = "[invalid technique index]";
  if (technique >= 0 && technique < (int)tech.size()) {
    return tech[technique].name;
  } else {
    return notfound;
  }
}

bool PEffect::findTechnique(const std::string &techname, int *technique)
{
  for (unsigned int i=0; i<tech.size(); i++) {
    if (tech[i].name == techname) {
      *technique = i;
      return true;
    }
  }
  return false;
}

bool PEffect::setCurrentTechnique(int technique)
{
  cur_tech = -1;
  if (technique >= 0 && technique < (int)tech.size()) {
    if (tech[technique].validated) {
      cur_tech = technique;
      return true;
    } else {
      con_printf("Effect::setCurrentTechnique(): technique not validated\n");
      return false;
    }
  } else {
    con_printf("Effect::setCurrentTechnique(): invalid technique index\n");
    return false;
  }
}

int PEffect::getCurrentTechnique()
{
  return cur_tech;
}

bool PEffect::setFirstValidTechnique()
{
  for (int i=0; i<getNumTechniques(); i++) {
    if (validateTechnique(i)) {
      setCurrentTechnique(i);
      return true;
    }
  }
  return false;
}

bool PEffect::renderBegin(int *numPasses, PSSTexture &sstex)
{
  if (cur_tech == -1) return false;

  fx_technique_s *curtech = &tech[cur_tech];

  if (!curtech->textures_ready) {
    for (unsigned int i=0; i<curtech->pass.size(); i++) {
      int texindex = curtech->pass[i].rs.texunit[0].texindex;
      if (texindex != -1 && !tex[texindex].texobject)
        tex[texindex].texobject = sstex.loadTexture(tex[texindex].filename);
    }
    curtech->textures_ready = true;
  }

  if (numPasses) *numPasses = curtech->pass.size();

  return true;
}

// utility func to set GL state
void PEffect::migrateRenderState(fx_renderstate_s *rs_old, fx_renderstate_s *rs_new)
{
  if (rs_old->depthtest != rs_new->depthtest) {
    if (rs_new->depthtest)  glEnable(GL_DEPTH_TEST);
    else                    glDisable(GL_DEPTH_TEST);
  }

  if (rs_old->lighting != rs_new->lighting) {
    if (rs_new->lighting)   glEnable(GL_LIGHTING);
    else                    glDisable(GL_LIGHTING);
  }

  if (rs_old->lightmodeltwoside != rs_new->lightmodeltwoside) {
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, rs_new->lightmodeltwoside ? GL_TRUE : GL_FALSE);
  }

  if (rs_old->alphatest.func != rs_new->alphatest.func ||
    (rs_new->alphatest.func != GL_ALWAYS &&
      rs_old->alphatest.ref != rs_new->alphatest.ref)) {
    if (rs_old->alphatest.func == GL_ALWAYS) {
      // enabling alpha test
      glEnable(GL_ALPHA_TEST);
      glAlphaFunc(rs_new->alphatest.func, rs_new->alphatest.ref);
    } else if (rs_new->alphatest.func == GL_ALWAYS) {
      // disabling alpha test
      glDisable(GL_ALPHA_TEST);
    } else {
      // changing alpha test
      glAlphaFunc(rs_new->alphatest.func, rs_new->alphatest.ref);
    }
  }

  if (rs_old->cullface != rs_new->cullface) {
    if (rs_old->cullface == CULLFACE_NONE) {
      glEnable(GL_CULL_FACE);
      glCullFace(rs_new->cullface == CULLFACE_CW ? GL_BACK : GL_FRONT);
    } else if (rs_new->cullface == CULLFACE_NONE) {
      glDisable(GL_CULL_FACE);
    } else {
      glCullFace(rs_new->cullface == CULLFACE_CW ? GL_BACK : GL_FRONT);
    }
  }

  if (rs_old->blendmode != rs_new->blendmode) {
    switch (rs_new->blendmode) {
    default:
      glBlendFunc(GL_ONE,GL_ZERO);
      break;
    case BLEND_ADD:
      glBlendFunc(GL_ONE,GL_ONE);
      break;
    case BLEND_MULTIPLY:
      glBlendFunc(GL_DST_COLOR,GL_SRC_COLOR);
      break;
    case BLEND_ALPHA:
      glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
      break;
    case BLEND_PREMULTALPHA:
      glBlendFunc(GL_ONE,GL_ONE_MINUS_SRC_ALPHA);
      break;
    }
  }

  int texindex = rs_new->texunit[0].texindex;
  if (texindex != -1 && tex[texindex].texobject)
    tex[texindex].texobject->bind();
  else
    PTexture::unbind();
}

void PEffect::renderPass(int pass)
{
  // assume pass is valid

  if (pass == 0)
    migrateRenderState(&def_rs,
            &tech[cur_tech].pass[0].rs);
  else
    migrateRenderState(&tech[cur_tech].pass[pass-1].rs,
            &tech[cur_tech].pass[pass].rs);
}

void PEffect::renderEnd()
{
  migrateRenderState(&tech[cur_tech].pass.back().rs,
          &def_rs);
}






