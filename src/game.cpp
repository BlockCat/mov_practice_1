#include "template.h"

#define PARTICLES		18000
#define BALLRADIUS		1.7f
#define SPRITESIZE		15

using namespace Tmpl8;

// calculate accurate square roots of four scalars in an __m128
static __forceinline float fastsqrtf(float v)
{
	const __m128 v1 = _mm_load_ss(&v);
	const __m128 v2 = _mm_rsqrt_ss(v1);
	float final;
	_mm_store_ss(&final, _mm_mul_ps(v1, v2));
	return final;
}

class TreeNode;

Tmpl8::vec3 p[PARTICLES], l[PARTICLES], pt[PARTICLES];
float pr[PARTICLES];
int pidx[PARTICLES], pleft[PARTICLES], pright[PARTICLES];
int sx[PARTICLES], sy[PARTICLES];
Tmpl8::vec3 gmin(-115, -55, -45), gmax(115, 2460, 45);
Surface back(SCRWIDTH, SCRHEIGHT);
Surface ikea("assets/back.png");
TreeNode* node, *root;
int nodeidx;
int interlace = 0;
float r = 270, r2 = 270;
Sprite* sprite[4] = {
	new Sprite(new Surface(SPRITESIZE, SPRITESIZE), 1),
	new Sprite(new Surface(SPRITESIZE, SPRITESIZE), 1),
	new Sprite(new Surface(SPRITESIZE, SPRITESIZE), 1),
	new Sprite(new Surface(SPRITESIZE, SPRITESIZE), 1) };

static long ballCount[256], index[256];
int loffs1[SPRITESIZE * SPRITESIZE], loffs2[SPRITESIZE * SPRITESIZE], scale[SPRITESIZE * SPRITESIZE];
int col[4][SPRITESIZE * SPRITESIZE];

void InitLens()
{
	Pixel* p1 = sprite[0]->GetSurface()->GetBuffer();
	Pixel* p2 = sprite[1]->GetSurface()->GetBuffer();
	Pixel* p3 = sprite[2]->GetSurface()->GetBuffer();
	Pixel* p4 = sprite[3]->GetSurface()->GetBuffer();
	memset(p1, 0, SPRITESIZE * SPRITESIZE * 4);
	memset(p2, 0, SPRITESIZE * SPRITESIZE * 4);
	memset(p3, 0, SPRITESIZE * SPRITESIZE * 4);
	memset(p4, 0, SPRITESIZE * SPRITESIZE * 4);
	for (int y = 0; y < SPRITESIZE; y++) for (int x = 0; x < SPRITESIZE; x++)
	{
		float dx = (float)x - (SPRITESIZE / 2);
		float dy = (float)y - (SPRITESIZE / 2);
		float l = sqrtf(dx * dx + dy * dy);
		float d = (SPRITESIZE / 2) - l;		
		//d = max(0, d);
		if (d < 0) d = 0;
		if (d > 0)
		{
			dx /= l;
			dy /= l;
			d /= (SPRITESIZE / 2);
			l /= (SPRITESIZE / 2);
			int x1 = SPRITESIZE / 2 + dx * (SPRITESIZE / 2) * cosf(d * PI / 2);
			int y1 = SPRITESIZE / 2 + dy * (SPRITESIZE / 2) * cosf(d * PI / 2);
			int x2 = SPRITESIZE / 2 + dx * (SPRITESIZE / 2) * (1 - sinf(d * PI / 2));
			int y2 = SPRITESIZE / 2 + dy * (SPRITESIZE / 2) * (1 - sinf(d * PI / 2));
			loffs1[x + y * SPRITESIZE] = x1 + y1 * SCRWIDTH;
			loffs2[x + y * SPRITESIZE] = x2 + y2 * SCRWIDTH;
			int scale = 12 + (int)(20 * tanf(d * d * 0.8f));
			col[0][x + y * SPRITESIZE] = p1[x + y * SPRITESIZE] = ScaleColor(0xff0000, scale);
			col[1][x + y * SPRITESIZE] = p2[x + y * SPRITESIZE] = ScaleColor(0x00ff00, scale);
			col[2][x + y * SPRITESIZE] = p3[x + y * SPRITESIZE] = ScaleColor(0x0000ff, scale);
			col[3][x + y * SPRITESIZE] = p4[x + y * SPRITESIZE] = ScaleColor(0xffffff, scale);
		}
		else
		{
			loffs1[x + y * SPRITESIZE] = 0;
			loffs2[x + y * SPRITESIZE] = 0;
		}
	}
	sprite[0]->InitializeStartData();
	sprite[1]->InitializeStartData();
	sprite[2]->InitializeStartData();
	sprite[3]->InitializeStartData();
}

inline void radix0(const unsigned long N, const long *source, long *dest)
{
	unsigned int i;
	memset(ballCount, 0, sizeof(ballCount));
	for (i = 0; i < N; i++) ballCount[source[i << 1] >> 24]++;
	index[0] = 0;
	for (i = 1; i < 256; i++) index[i] = index[i - 1] + ballCount[i - 1];
	for (i = 0; i < N; i++)
	{
		const unsigned int idx = index[source[i << 1] >> 24]++ << 1;
		dest[idx] = source[i << 1];
		dest[idx + 1] = source[(i << 1) + 1];
	}
}

inline void radix1(const unsigned long N, const long *source, long *dest)
{
	unsigned int i;
	memset(ballCount, 0, sizeof(ballCount));
	for (i = 0; i < N; i++) ballCount[source[i << 1] & 255]++;
	index[0] = 0;
	for (i = 1; i < 256; i++) index[i] = index[i - 1] + ballCount[i - 1];
	for (i = 0; i < N; i++)
	{
		const unsigned int idx = index[source[i << 1] & 255]++ << 1;
		dest[idx] = source[i << 1];
		dest[idx + 1] = source[(i << 1) + 1];
	}
}

inline void radix8(const int bit, const unsigned long N, const long *source, long *dest)
{
	unsigned int i;
	memset(ballCount, 0, sizeof(ballCount));
	for (i = 0; i < N; i++) ballCount[(source[i << 1] >> bit) & 255]++;
	index[0] = 0;
	for (i = 1; i < 256; i++) index[i] = index[i - 1] + ballCount[i - 1];
	for (i = 0; i < N; i++)
	{
		const unsigned int idx = index[(source[i << 1] >> bit) & 255]++ << 1;
		dest[idx] = source[i << 1];
		dest[idx + 1] = source[(i << 1) + 1];
	}
}

unsigned int source[PARTICLES * 2], temp[PARTICLES * 2];

void Sort()
{
	radix1(PARTICLES, (long*)source, (long*)temp);
	radix8(8, PARTICLES, (long*)temp, (long*)source);
}

class TreeNode
{
public:
	void SubDiv(Tmpl8::vec3 bmin, Tmpl8::vec3 bmax)
	{
		Tmpl8::vec3 smin[64], smax[64];
		TreeNode* snode[64];
		smin[0] = bmin, smax[0] = bmax, snode[0] = this;
		int stackptr = 1;
		while (stackptr)
		{
			const Tmpl8::vec3 bmin = smin[--stackptr];
			const Tmpl8::vec3 bmax = smax[stackptr];
			TreeNode* curr = snode[stackptr];
			int axis = 0;
			const float ex[3] = { bmax.x - bmin.x, bmax.y - bmin.y, bmax.z - bmin.z };
			if (ex[1] > ex[0]) axis = 1; else axis = 0;
			if (ex[2] > ex[axis]) axis = 2;
			curr->left = &node[nodeidx], nodeidx += 2;
			int lcount = 0, rcount = 0;
			const float splitpos = (bmax.cell[axis] + bmin.cell[axis]) * 0.5f;
			for (int i = 0; i < curr->count; i++)
			{
				int idx = pidx[i + curr->first];
				if (p[idx].cell[axis] < splitpos) pleft[lcount++] = pidx[i + curr->first]; else pright[rcount++] = pidx[i + curr->first];
			}
			memcpy(&pidx[curr->first], pleft, lcount * 4);
			memcpy(&pidx[curr->first + lcount], pright, rcount * 4);
			curr->left->first = curr->first;
			curr->left->count = lcount;
			curr->left->left = 0;
			(curr->left + 1)->first = curr->first + lcount;
			(curr->left + 1)->count = rcount;
			(curr->left + 1)->left = 0;
			Tmpl8::vec3 lmax = bmax, rmin = bmin;
			lmax.cell[axis] = rmin.cell[axis] = splitpos;
			if (lcount > 8) snode[stackptr] = curr->left, smin[stackptr] = bmin, smax[stackptr++] = lmax;
			if (rcount > 8) snode[stackptr] = curr->left + 1, smin[stackptr] = rmin, smax[stackptr++] = bmax;
			curr->axis = axis;
			curr->splitpos = splitpos;
		}
	}
	// data members
	TreeNode* left;
	union
	{
		float splitpos;
		int first;
	};
	int count;
	int axis;
};

void BuildTree()
{
	root = &node[0];
	nodeidx = 1;
	for (int i = 0; i < PARTICLES; i++) pidx[i] = i;
	root->first = 0;
	root->count = PARTICLES;
	root->SubDiv(gmin, gmax);
}

void Game::Init()
{
	for (int i = 0; i < PARTICLES; i++)
	{
		p[i].x = l[i].x = Rand(15) - 100;
		p[i].z = l[i].z = Rand(30) - 15;
		p[i].y = l[i].y = 2400 - Rand(2014);
		pr[i] = BALLRADIUS - Rand(0.1f);
		sx[i] = 0, sy[i] = 0;
	}
	back.Clear(0);
	node = (TreeNode*)MALLOC64(sizeof(TreeNode) * PARTICLES * 2);
	InitLens();
	BuildTree();
}

void Game::Tick(float a_DT)
{
	int start = GetTickCount();
	screen->Clear(0);
	Pixel* bptr = back.GetBuffer();
	ikea.CopyTo(&back, 0, 0);
	if (GetAsyncKeyState(VK_LEFT)) { r -= 1.8f; if (r < 0) r += 360; }
	if (GetAsyncKeyState(VK_RIGHT)) { r += 1.8f; if (r > 360) r -= 360; }
	if (GetAsyncKeyState(VK_UP)) { r2 -= 1.8f; if (r2 < 0) r2 += 360; }
	if (GetAsyncKeyState(VK_DOWN)) { r2 += 1.8f; if (r2 > 360) r2 -= 360; }
	float sinr = sin(r * PI / 180);
	float cosr = cos(r * PI / 180);
	float sinr2 = sin(r2 * PI / 180);
	float cosr2 = cos(r2 * PI / 180);
	// verlet: update positions, apply forces
	for (int j = 0; j < 2; j++)
	{
		for (int i = 0; i < PARTICLES; i++)
		{
			Tmpl8::vec3 _p = p[i];
			p[i] += (p[i] - l[i]) - Tmpl8::vec3(-0.015f * cosr2, -0.015f * sinr2, 0);
			l[i] = _p;
		}
		// verlet: satisfy constraints
		BuildTree();
#pragma omp parallel for schedule(dynamic,4096)
		for (int i = 0; i < PARTICLES; i++)
		{
			TreeNode* node = root;
			TreeNode* stack[64];
			int stackptr = 0;
			while (1)
			{
				if (node->left)
				{
					int axis = node->axis;
					if ((p[i].cell[axis] - 2 * BALLRADIUS) < node->splitpos)
					{
						if ((p[i].cell[axis] + 2 * BALLRADIUS) > node->splitpos) stack[stackptr++] = node->left + 1;
						node = node->left;
					}
					else if ((p[i].cell[axis] + 2 * BALLRADIUS) > node->splitpos) node = node->left + 1;
					continue;
				}
				else
				{
					// move away from other balls
					for (int k = 0; k < node->count; k++)
					{
						const int idx = pidx[node->first + k];
						if (idx <= i) continue;
						const Tmpl8::vec3 d = p[idx] - p[i];
						const float dist = sqrtf(d.x * d.x + d.y * d.y + d.z * d.z);
						const float mindist = pr[i] + pr[idx];
						if (dist < mindist)
						{
							const float intrusion = mindist - dist;
							const float il = 0.5f / dist;
							const Tmpl8::vec3 m = d * intrusion * il;
							p[idx] += m, p[i] -= m;
						}
					}
				}
				if (!stackptr) break;
				node = stack[--stackptr];
			}
			// keep away from  mouse object
			Tmpl8::vec3 mpos(gmin.x + ((float)mx / SCRWIDTH) * (gmax.x - gmin.x),
				-gmin.y + ((float)my / SCRHEIGHT) * (gmin.y + gmin.y),
				0);
			for (int a = 0; a < 3; a++)
			{
				mpos.cell[a] = (mpos.cell[a] < gmin.cell[a]) ? gmin.cell[a] : mpos.cell[a];
				mpos.cell[a] = (mpos.cell[a] > gmax.cell[a]) ? gmax.cell[a] : mpos.cell[a];
			}
			const Tmpl8::vec3 d = mpos - p[i];
			const float dist = d.x * d.x + d.y * d.y;
			const float mindist = pr[i] + 15;
			if (dist < (mindist * mindist))
			{
				const float l = fastsqrtf(dist);
				const float intrusion = mindist - l;
				const float il = 1.0f / l;
				const Tmpl8::vec3 m = d * intrusion * il;
				p[i] -= m;
			}
			// keep away from floor and sides
			for (int a = 0; a < 3; a++)
			{
				if ((p[i].cell[a] - pr[i]) < gmin.cell[a]) p[i].cell[a] = gmin.cell[a] + pr[i];
				if ((p[i].cell[a] + pr[i]) > gmax.cell[a]) p[i].cell[a] = gmax.cell[a] - pr[i];
			}
		}
	}
	// add to sort array
	for (int i = 0; i < PARTICLES; i++)
	{
		const float rx = p[i].x * sinr + p[i].z * cosr;
		const float rz = p[i].x * cosr - p[i].z * sinr;
		const float frx = rx * sinr2 + p[i].y * cosr2;
		const float fry = rx * cosr2 - p[i].y * sinr2;
		pt[i] = Tmpl8::vec3(frx, fry, rz);
		source[i << 1] = (int)(100 * (500 - rz));
		source[(i << 1) + 1] = i;
	}
	Sort();
	// render
	for (int i = 0; i < PARTICLES; i++)
	{
		int idx = source[(i << 1) + 1];
		const float reciz = 1.0f / (pt[idx].z + 500);
		const int sx = (int)((pt[idx].x * 2000) * reciz + SCRWIDTH / 2);
		int sy = (int)(SCRHEIGHT / 2 - (pt[idx].y * 2000) * reciz);
		sprite[idx & 3]->Draw(&back, sx - 8, sy - 8);
	}
	// finalize
	back.CopyTo(screen, 0, 0);
	int elapsed = GetTickCount() - start;
	char t[64];
	sprintf(t, "%i ms", elapsed);
	Pixel* b = screen->GetBuffer();
	memset(b, 0, 4 * 1024);
	for (int i = 0; i < 10; i++) memset(b + i * screen->GetPitch(), 0, 50 * 4);
	screen->Print(t, 8, 1, 0xffffff);
	if ((mx >= 0) && (mx < SCRWIDTH)) screen->Line(mx, 0, mx, SCRHEIGHT - 1, 0xff0000);
	if ((my >= 0) && (my < SCRHEIGHT)) screen->Line(0, my, SCRWIDTH - 1, my, 0xff0000);
}
