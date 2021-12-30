// ****************************************************************************
// Oct Tree
// ****************************************************************************
namespace SCENETREE
{
	#define MAXDEPTH 5

	struct octree
	{
		boundingbox aabb;

		//std::vector<int> triinds;
		std::vector<int> ents;

		octree* children[8];

		octree()
		{
			for (int i = 0; i < 8; i++)
				children[i] = 0;
		}
	};
	octree* findnode(octree* oct, const ray3& ray, int depth = 0)
	{
		for (int ii = 0; ii < 8; ii++)
		{
			if (oct->children[ii] && oct->children[ii]->aabb.intersection(ray).first)
			{
				if (depth + 1 == MAXDEPTH)
					return oct->children[ii];
				return findnode(oct->children[ii], ray, depth + 1);
			}
		}
		return 0;
	}
	void findnodes(octree* oct, const ray3& ray, std::vector<octree*>& result, int depth = 0)
	{
		for (int ii = 0; ii < 8; ii++)
		{
			if (oct->children[ii] && oct->children[ii]->aabb.intersection(ray).first)
			{
				if (depth + 1 == MAXDEPTH)
					result.push_back(oct->children[ii]);
				else
					findnodes(oct->children[ii], ray, result, depth + 1);
			}
		}
	}
	octree* findnode(octree* oct, const vector3& p, int depth = 0)
	{
		for (int ii = 0; ii < 8; ii++)
		{
			if (oct->children[ii]->aabb.intersection(p))
			{
				if (depth + 1 == MAXDEPTH)
					return oct->children[ii];
				return findnode(oct->children[ii], p, depth + 1);
			}
		}
		return 0;
	}
	void findnodes(octree* oct, const boundingbox& aabb, int depth, std::vector<octree*>& result)
	{
		for (int i = 0; i < 8; i++)
		{
			if (oct->children[i] && oct->children[i]->aabb.intersection(aabb))
			{
				if (depth + 1 == MAXDEPTH)
					result.push_back(oct->children[i]);
				else
					findnodes(oct->children[i], aabb, depth + 1, result);
			}
		}
	}
	void walk(octree* ot, int depth)
	{
		if (depth > MAXDEPTH)
			return;
		
		for (int i = 0; i < 8; i++)
		{
			if (ot->children[i])
				walk(ot->children[i], depth + 1);
		}
	}
	void clear(octree* ot)
	{
		for (int i = 0; i < 8; i++)
		{
			if (ot->children[i])
				clear(ot->children[i]);
		}
		delete(ot);
	}
	void build(octree* ot, const boundingbox& aabb, int depth)
	{
		ot->aabb = aabb;

		if (depth < MAXDEPTH)
		{
			const vec3& a = aabb.a;
			const vec3& b = aabb.b;
			vec3 m = (aabb.a + aabb.b) / 2;
			depth ++;

			ot->children[0] = new octree();
			build(ot->children[0], boundingbox(a, m), depth);

			ot->children[1] = new octree();
			build(ot->children[1], boundingbox(vec3(m.x, a.y, a.z), vec3(b.x, m.y, m.z)), depth);

			ot->children[2] = new octree();
			build(ot->children[2], boundingbox(vec3(a.x, m.y, a.z), vec3(m.x, b.y, m.z)), depth);

			ot->children[3] = new octree();
			build(ot->children[3], boundingbox(vec3(a.x, a.y, m.z), vec3(m.x, m.y, b.z)), depth);

			ot->children[4] = new octree();
			build(ot->children[4], boundingbox(m, b), depth);

			ot->children[5] = new octree();
			build(ot->children[5], boundingbox(vec3(a.x, m.y, m.z), vec3(m.x, b.y, b.z)), depth);

			ot->children[6] = new octree();
			build(ot->children[6], boundingbox(vec3(m.x, a.y, m.z), vec3(b.x, m.y, b.z)), depth);

			ot->children[7] = new octree();
			build(ot->children[7], boundingbox(vec3(m.x, m.y, a.z), vec3(b.x, b.y, m.z)), depth);
		}
	}
	inline void tri_aabb(boundingbox& aabb, crvec p1, crvec p2, crvec p3)
	{
		aabb.a.x = _MIN(p1.x, _MIN(p2.x, p3.x));
		aabb.b.x = _MAX(p1.x, _MAX(p2.x, p3.x));

		aabb.a.y = _MIN(p1.y, _MIN(p2.y, p3.y));
		aabb.b.y = _MAX(p1.y, _MAX(p2.y, p3.y));

		aabb.a.z = _MIN(p1.z, _MIN(p2.z, p3.z));
		aabb.b.z = _MAX(p1.z, _MAX(p2.z, p3.z));
	}
	void bindSM(octree* oct, submesh& sm)
	{
		PRINT("bindSM:");
		for (int i = 0; i < sm.tris.size(); i++)
		{
			int t1 = sm.tris[i].vertexIndex[0];
			int t2 = sm.tris[i].vertexIndex[1];
			int t3 = sm.tris[i].vertexIndex[2];

			vertex& p1 = sm.vertices[t1];
			vertex& p2 = sm.vertices[t2];
			vertex& p3 = sm.vertices[t3];

			boundingbox aabb;
			tri_aabb(aabb, p1.p, p2.p, p3.p);

			std::vector<octree*> result;
			findnodes(oct, aabb, 0, result);
			PRINTV(result.size());

			for (auto& it : result)
			{
				//it->triinds.push_back(i);
				PRINT("bindSM:" << (it->aabb.a.x) << "," << i);
			}
		}
	}

	// ================================================
	struct octentity_t {
		boundingbox aabb;
		vec3 pos;
		vec3 scl;
		vec3 eulers;
		char meshmd5[33];
		octentity_t() {}
		octentity_t(boundingbox& _aabb, const char* _md5)
		{
			aabb = _aabb;
			strcpy(meshmd5, _md5);
		}
	};
	struct scenenode_t
	{
		char name[128];
		std::vector<octentity_t> elist;
		std::vector<scenenode_t*> children;

		void clear(scenenode_t* scenenode)
		{
			for(auto& it : scenenode->children)
				clear(it);
			
			if(&g_nodetree != scenenode)
				delete scenenode;
			else
			{
				scenenode->elist.clear();
				scenenode->children.clear();
			}
		}
	}g_nodetree;

	std::vector<octentity_t>	g_octentlist;					// enity list

	// 圆柱体
	struct CylinderFeature
	{
		float m_height;
		float m_radius;
	};
	// 球体
	struct SphereFeature
	{
		float m_radius;
	};
	// 方盒
	struct BoxFeature
	{
		vec3 m_axisX;
		vec3 m_axisY;
		vec3 m_axisZ;
	};
	struct prmfeature_t {
		char buff[64];
		short size;
		short type;
	};
	unordered_map<std::string, prmfeature_t> g_prmfeature;		// primative feature

	// -------------------------------------------------
	void bindAABB(octree* oct, octentity_t& ent)
	{
		std::vector<octree*> result;
		findnodes(oct, ent.aabb, 0, result);

		for (auto it : result)
		{
			it->ents.push_back(g_octentlist.size() - 1);
			//pointi(aabb.a.x, aabb.a.z, 4, 0xFF0000FF);
		}
	}

	// ------------------------------------------------
	// read
	// ------------------------------------------------
	void readtxt(scenenode_t* parent, const char* name, FILE* file)
	{
		//PRINT("readtxt");
		scenenode_t* node = new scenenode_t;
		strcpy_s(node->name, name);
		char buff[256];
		while (fgets(buff, 256, file) != NULL)
		{
		//	PRINT(buff);

			if (buff[0] == '{')
			{
				//PRINT("{-");
				char name[128];
				sscanf(buff, "{%s\n", name);
				//PRINTV(name);
				readtxt(node, name, file);
			}
			else if (buff[0] == '}')
			{
				//PRINT("-}");
				
				break;
			}
			else
			{
				//PRINT("ent")
				octentity_t ent;
				sscanf(buff, "%f,%f,%f %f,%f,%f %f,%f,%f %f,%f,%f %f,%f,%f %s\n",
					&ent.aabb.a.x, &ent.aabb.a.y, &ent.aabb.a.z,
					&ent.aabb.b.x, &ent.aabb.b.y, &ent.aabb.b.z,
					&ent.pos.x, &ent.pos.y, &ent.pos.z,
					&ent.scl.x, &ent.scl.y, &ent.scl.z,
					&ent.eulers.x, &ent.eulers.y, &ent.eulers.z,
					ent.meshmd5
				);
				ent.pos = ent.aabb.getcenter();

				node->elist.push_back(ent);
			}
		}
		parent->children.push_back(node);
	}
	bool readtxt(const std::string& filename)
	{
		PRINT("readtxt ...");
		bool ret = false;

		FILE* file;
		if (fopen_s(&file, filename.c_str(), "r") == 0)
		{
			readtxt(&g_nodetree, "ROOT", file);

			fclose(file);
			ret = true;
		}
		return ret;
	}
	void readmesh(submesh& sm)
	{
		for (int i = 0; i < 5; i++)
		{
			std::string filename = "C:/Users/18858/Documents/WORK/dattest/" + to_string(i) + ".txt";
			if (!readsubmesh(sm, filename, 1.0F))
				break;
		}
	}

	void read_scenenode(scenenode_t* parent, FILE* file)
	{
		scenenode_t* node = new scenenode_t;
		fread(node->name, 128, 1, file);
		//PRINTV(node->name);
		int entnum;
		fread(&entnum, 4, 1, file);
		for (int i = 0; i < entnum; i++)
		{
			octentity_t ent;
			fread(&ent, sizeof(ent), 1, file);
			node->elist.push_back(ent);

			drawrect(ent.aabb.a.x, ent.aabb.a.z, ent.aabb.b.x, ent.aabb.b.z, 0xFFFF00FF);
		}

		int cnum;
		fread(&cnum, 4, 1, file);
		for (int i = 0; i < cnum; i++)
		{
			read_scenenode(node, file);
		}
		parent->children.push_back(node);
	}
	void read_nodetree(const std::string& filename)
	{
		g_nodetree.clear(&g_nodetree);

		PRINT("read nodetree ...");
		FILE* file;
		if (fopen_s(&file, filename.c_str(), "rb") == 0)
		{
			read_scenenode(&g_nodetree, file);

			fclose(file);
		}
	}
	void read(FILE* file, octree* ot, const boundingbox& aabb, int depth)
	{
		fread(&ot->aabb, sizeof(ot->aabb), 1, file);

		int entnum;
		fread(&entnum, 4, 1, file);
		for (int i = 0; i < entnum; i++) {
			int entind;
			fread(&entind, 4, 1, file);
			ot->ents.push_back(entind);
			octentity_t& ent = g_octentlist[entind];

			unordered_map<std::string, prmfeature_t>::iterator it = g_prmfeature.find(ent.meshmd5);
			if (it != g_prmfeature.end())
			{
				prmfeature_t& ft = it->second;
				float* pfdat = (float*)(ft.buff);
				float height = *pfdat;
				float rad = *(pfdat + 1);
				//drawrect(ent.pos.x, ent.pos.z, ent.pos.x + height, ent.pos.z + rad, 0xFFFF00FF);
				drawrect(ent.aabb.a.x, ent.aabb.a.z, ent.aabb.b.x, ent.aabb.b.z, 0xFFFF00FF);
				//PRINTV(ent.meshmd5);
				//PRINTV(ft.type);
				//PRINTV(ft.size);
			}
		}
		if (depth < MAXDEPTH)
		{
			const vec3& a = aabb.a;
			const vec3& b = aabb.b;
			vec3 m = (aabb.a + aabb.b) / 2;
			depth++;
			ot->children[0] = new octree();
			read(file, ot->children[0], boundingbox(a, m), depth);

			ot->children[1] = new octree();
			read(file, ot->children[1], boundingbox(vec3(m.x, a.y, a.z), vec3(b.x, m.y, m.z)), depth);

			ot->children[2] = new octree();
			read(file, ot->children[2], boundingbox(vec3(a.x, m.y, a.z), vec3(m.x, b.y, m.z)), depth);

			ot->children[3] = new octree();
			read(file, ot->children[3], boundingbox(vec3(a.x, a.y, m.z), vec3(m.x, m.y, b.z)), depth);

			ot->children[4] = new octree();
			read(file, ot->children[4], boundingbox(m, b), depth);

			ot->children[5] = new octree();
			read(file, ot->children[5], boundingbox(vec3(a.x, m.y, m.z), vec3(m.x, b.y, b.z)), depth);

			ot->children[6] = new octree();
			read(file, ot->children[6], boundingbox(vec3(m.x, a.y, m.z), vec3(b.x, m.y, b.z)), depth);

			ot->children[7] = new octree();
			read(file, ot->children[7], boundingbox(vec3(m.x, m.y, a.z), vec3(b.x, b.y, m.z)), depth);
		}
	}

	void read(octree* oct, boundingbox& aabb, const std::string& filename)
	{
		PRINT("read scene ...")
			int tk = GetTickCount();
		FILE* file;
		if (fopen_s(&file, filename.c_str(), "rb") == 0)
		{
			{// enity list
				int entnum;
				fread(&entnum, 4, 1, file);
				PRINTV(entnum);
				for (int i = 0; i < entnum; i++)
				{
					octentity_t ent;
					fread(&ent, sizeof(ent), 1, file);
					g_octentlist.push_back(ent);
				}
			}

			// octree
			read(file, oct, aabb, 0);

			fclose(file);
		}
		PRINT("scene entity list and octree = " << GetTickCount() - tk);
	}

	void read_prmfeature(const std::string& filename)
	{
		PRINT("read prmfeature ...")
			int tk = GetTickCount();
		FILE* file;
		if (fopen_s(&file, filename.c_str(), "rb") == 0)
		{
			{
				int num;
				fread(&num, sizeof(num), 1, file);
				PRINTV(num);
				for (int i = 0; i < num; i++)
				{
					char md5buff[33];
					fread(md5buff, sizeof(md5buff), 1, file);

					prmfeature_t pf;
					pf.type = 1;
					pf.size = sizeof(CylinderFeature);
					//PRINTV(pf.size)
					fread(pf.buff, pf.size, 1, file);
					g_prmfeature[md5buff] = pf;
				}
			}
			{
				int num;
				fread(&num, sizeof(num), 1, file);
				PRINTV(num);
				for (int i = 0; i < num; i++)
				{
					char md5buff[33];
					fread(md5buff, sizeof(md5buff), 1, file);

					prmfeature_t pf;
					pf.type = 2;
					pf.size = sizeof(SphereFeature);
					//PRINTV(pf.size)
					fread(pf.buff, pf.size, 1, file);

					g_prmfeature[md5buff] = pf;
				}
			}
			{
				int num;
				fread(&num, sizeof(num), 1, file);
				PRINTV(num);
				for (int i = 0; i < num; i++)
				{
					char md5buff[33];
					fread(md5buff, sizeof(md5buff), 1, file);

					prmfeature_t pf;
					pf.type = 3;
					pf.size = sizeof(BoxFeature);
					//PRINTV(pf.size)
					fread(pf.buff, pf.size, 1, file);

					g_prmfeature[md5buff] = pf;
				}
			}
			fclose(file);
		}
		PRINT("Primitive list and construction = " << GetTickCount() - tk);
	}
	// ------------------------------------------------
	// save
	// ------------------------------------------------
	void saveoct(FILE* file, octree* ot, int depth)
	{
		fwrite(&ot->aabb, sizeof(ot->aabb), 1, file);

		int entnum = ot->ents.size();
		fwrite(&entnum, 4, 1, file);
	
		for (int i = 0; i < entnum; i++)
			fwrite(&ot->ents[i], sizeof(ot->ents[i]), 1, file);

		if (depth < MAXDEPTH)
			for (int i = 0; i < 8; i++)
			{
				if (ot->children[i])
					saveoct(file, ot->children[i], depth + 1);
			}
	}

	void save(octree* oct, const std::string& filename)
	{
		PRINT("save scene ...");
		FILE* file;
		if (fopen_s(&file, filename.c_str(), "wb") == 0)
		{
			{
				int entnum = g_octentlist.size();
				PRINTV(entnum);
				fwrite(&entnum, 4, 1, file);
				for (int i = 0; i < entnum; i++)
				{
					fwrite(&g_octentlist[i], sizeof(g_octentlist[i]), 1, file);
				}
			}

			//saveoct(file, oct, 0);

			fclose(file);
		}
	}
	void save_scenenode(scenenode_t* node, FILE* file)
	{
		if (&g_nodetree != node)
		{
			//PRINT("save_scenenode!");
			fwrite(node->name, 128, 1, file);
			//PRINTV(node->name);
			int entnum = node->elist.size();
			fwrite(&entnum, 4, 1, file);
			for (int i = 0; i < entnum; i++)
			{
				fwrite(&node->elist[i], sizeof(node->elist[i]), 1, file);
			}
		}
		int cnum = node->children.size();
		fwrite(&cnum, 4, 1, file);
		for (auto& it : node->children)
		{
			save_scenenode(it, file);
		}
	}
	void save_nodetree(const std::string& filename)
	{
		PRINT("save nodetree ...");
		FILE* file;
		if (fopen_s(&file, filename.c_str(), "wb") == 0)
		{
			save_scenenode(&g_nodetree, file);

			fclose(file);
		}

	}
	void main()
	{
		read_prmfeature("C:/Users/18858/Documents/WORK/dattest/prm.fue");

		boundingbox aabb(vec3(0, 0, 0), vec3(1024, 512, 1024));
		{
			//octree* ot = new octree();

			//build(ot, aabb, 0);

			readtxt("C:/Users/18858/Documents/WORK/dattest/aabb.txt");

			//save(ot, "C:/Users/18858/Documents/WORK/dattest/scene.sce");
			save_nodetree("C:/Users/18858/Documents/WORK/dattest/nodetree.sce");

		//	clear(ot);
		}
		{
			octree* ot = new octree();

			read(ot, aabb, "C:/Users/18858/Documents/WORK/dattest/scene.sce");
			//read_nodetree("C:/Users/18858/Documents/WORK/dattest/nodetree.sce");
			//clear(ot);
		}
	}
}

// ****************************************************************************
// scene
// ****************************************************************************
void scene()
{
	SCENETREE::main();
}
void phg_test() {
	SCENETREE::main();
}