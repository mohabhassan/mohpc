#include <Shared.h>
#include <MOHPC/Assets/Formats/BSP.h>
#include "BSP_Curve.h"

#include "../../../Common/VectorPrivate.h"
#include <Eigen/Geometry>

using namespace MOHPC;
using namespace BSPData;

struct curveGrid_t
{
public:
	Vertice ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE];
};

static void LerpDrawVert(const BSPData::Vertice* a, const BSPData::Vertice* b, BSPData::Vertice* out)
{
	out->xyz[0] = 0.5f * (a->xyz[0] + b->xyz[0]);
	out->xyz[1] = 0.5f * (a->xyz[1] + b->xyz[1]);
	out->xyz[2] = 0.5f * (a->xyz[2] + b->xyz[2]);

	out->st[0] = 0.5f * (a->st[0] + b->st[0]);
	out->st[1] = 0.5f * (a->st[1] + b->st[1]);

	out->lightmap[0] = 0.5f * (a->lightmap[0] + b->lightmap[0]);
	out->lightmap[1] = 0.5f * (a->lightmap[1] + b->lightmap[1]);

	out->color[0] = (uint8_t)(0.5f * ((float)a->color[0] + (float)b->color[0]));
	out->color[1] = (uint8_t)(0.5f * ((float)a->color[1] + (float)b->color[1]));
	out->color[2] = (uint8_t)(0.5f * ((float)a->color[2] + (float)b->color[2]));
	out->color[3] = (uint8_t)(0.5f * ((float)a->color[3] + (float)b->color[3]));
}

static void Transpose(int width, int height, BSPData::Vertice ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE])
{
	int		i, j;
	BSPData::Vertice temp;

	if (width > height) {
		for (i = 0; i < height; i++) {
			for (j = i + 1; j < width; j++) {
				if (j < height) {
					// swap the value
					temp = ctrl[j][i];
					ctrl[j][i] = ctrl[i][j];
					ctrl[i][j] = temp;
				}
				else {
					// just copy
					ctrl[j][i] = ctrl[i][j];
				}
			}
		}
	}
	else {
		for (i = 0; i < width; i++) {
			for (j = i + 1; j < height; j++) {
				if (j < width) {
					// swap the value
					temp = ctrl[i][j];
					ctrl[i][j] = ctrl[j][i];
					ctrl[j][i] = temp;
				}
				else {
					// just copy
					ctrl[i][j] = ctrl[j][i];
				}
			}
		}
	}

}

static void MakeMeshNormals(int width, int height, BSPData::Vertice ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE]) {
	int		i, j, k, dist;
	Vector3 normal;
	Vector3 sum;
	int		count = 0;
	Vector3	base;
	Vector3	delta;
	int		x, y;
	BSPData::Vertice *dv;
	Vector3		around[8], temp;
	bool	good[8];
	bool	wrapWidth, wrapHeight;
	double len;
	static	int	neighbors[8][2] = {
		{ 0,1 },{ 1,1 },{ 1,0 },{ 1,-1 },{ 0,-1 },{ -1,-1 },{ -1,0 },{ -1,1 }
	};

	wrapWidth = false;
	for (i = 0; i < height; i++)
	{
		delta = castVector(ctrl[i][0].xyz) - castVector(ctrl[i][width - 1].xyz);
		len = delta.squaredNorm();
		if (len > 1.0) {
			break;
		}
	}
	if (i == height) {
		wrapWidth = true;
	}

	wrapHeight = false;
	for (i = 0; i < width; i++) {
		delta = castVector(ctrl[i][0].xyz) - castVector(ctrl[i][height - 1].xyz);
		len = delta.squaredNorm();
		if (len > 1.0) {
			break;
		}
	}
	if (i == width) {
		wrapHeight = true;
	}


	for (i = 0; i < width; i++) {
		for (j = 0; j < height; j++) {
			count = 0;
			dv = &ctrl[j][i];
			base = castVector(dv->xyz);
			for (k = 0; k < 8; k++) {
				around[k] = castVector(vec3_zero);
				good[k] = false;

				for (dist = 1; dist <= 3; dist++) {
					x = i + neighbors[k][0] * dist;
					y = j + neighbors[k][1] * dist;
					if (wrapWidth) {
						if (x < 0) {
							x = width - 1 + x;
						}
						else if (x >= width) {
							x = 1 + x - width;
						}
					}
					if (wrapHeight) {
						if (y < 0) {
							y = height - 1 + y;
						}
						else if (y >= height) {
							y = 1 + y - height;
						}
					}

					if (x < 0 || x >= width || y < 0 || y >= height) {
						break;					// edge of patch
					}
					temp = castVector(ctrl[y][x].xyz) - base;
					if (temp.squaredNorm() == 0.f)
					{
						// degenerate edge, get more dist
						continue;
					}
					else
					{
						temp.normalize();
						good[k] = true;
						around[k] = temp;
						// good edge
						break;
					}
				}
			}

			sum = castVector(vec3_zero);
			for (k = 0; k < 8; k++) {
				if (!good[k] || !good[(k + 1) & 7]) {
					continue;	// didn't get two points
				}
				normal = around[(k + 1) & 7];
				normal.cross(around[k]);
				sum += normal;
				count++;
			}
			//if ( count == 0 ) {
			//	printf("bad normal\n");
			//}
			castVector(dv->normal) = sum;
			castVector(dv->normal).normalize();
		}
	}
}

static int MakeMeshIndexes(int width, int height, int32_t indexes[(MAX_GRID_SIZE - 1)*(MAX_GRID_SIZE - 1) * 2 * 3])
{
	int             i, j;
	int             numIndexes;
	int             w, h;
	//BSPData::Vertice *dv;
	//static BSPData::Vertice ctrl2[MAX_GRID_SIZE * MAX_GRID_SIZE];

	h = height - 1;
	w = width - 1;
	numIndexes = 0;
	for (i = 0; i < h; i++)
	{
		for (j = 0; j < w; j++)
		{
			int             v1, v2, v3, v4;

			// vertex order to be recognized as tristrips
			v1 = i * width + j + 1;
			v2 = v1 - 1;
			v3 = v2 + width;
			v4 = v3 + 1;

			indexes[numIndexes++] = v2;
			indexes[numIndexes++] = v3;
			indexes[numIndexes++] = v1;

			indexes[numIndexes++] = v1;
			indexes[numIndexes++] = v3;
			indexes[numIndexes++] = v4;
		}
	}

	/*
	// FIXME: use more elegant way
	for (i = 0; i < width; i++)
	{
		for (j = 0; j < height; j++)
		{
			dv = &ctrl2[j * width + i];
			*dv = ctrl[j][i];
		}
	}
	*/

	return numIndexes;
}

static void InvertCtrl(int width, int height, BSPData::Vertice ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE]) {
	int		i, j;
	BSPData::Vertice temp;

	for (i = 0; i < height; i++) {
		for (j = 0; j < width / 2; j++) {
			temp = ctrl[i][j];
			ctrl[i][j] = ctrl[i][width - 1 - j];
			ctrl[i][width - 1 - j] = temp;
		}
	}
}

static void InvertErrorTable(float errorTable[2][MAX_GRID_SIZE], int width, int height) {
	int		i;
	float	copy[2][MAX_GRID_SIZE];

	memcpy(copy, errorTable, sizeof(copy));

	for (i = 0; i < width; i++) {
		errorTable[1][i] = copy[0][i];	//[width-1-i];
	}

	for (i = 0; i < height; i++) {
		errorTable[0][i] = copy[1][height - 1 - i];
	}

}

static void PutPointsOnCurve(BSPData::Vertice ctrl[MAX_GRID_SIZE][MAX_GRID_SIZE],
	int width, int height)
{
	int			i, j;
	BSPData::Vertice prev, next;

	for (i = 0; i < width; i++) {
		for (j = 1; j < height; j += 2) {
			LerpDrawVert(&ctrl[j][i], &ctrl[j + 1][i], &prev);
			LerpDrawVert(&ctrl[j][i], &ctrl[j - 1][i], &next);
			LerpDrawVert(&prev, &next, &ctrl[j][i]);
		}
	}


	for (j = 0; j < height; j++) {
		for (i = 1; i < width; i += 2) {
			LerpDrawVert(&ctrl[j][i], &ctrl[j][i + 1], &prev);
			LerpDrawVert(&ctrl[j][i], &ctrl[j][i - 1], &next);
			LerpDrawVert(&prev, &next, &ctrl[j][i]);
		}
	}
}

void BSPReader::CreateSurfaceGridMesh(int32_t width, int32_t height, BSPData::Vertice *ctrl, int32_t numIndexes, int32_t *indexes, BSPData::Surface* grid)
{
	BSPData::Vertice *vert;
	Vector3 tmpVec;

	grid->indexes.resize(numIndexes);
	grid->vertices.resize(width * height);

	uint32_t *outIndexes = grid->indexes.data();
	for (size_t i = 0; i < numIndexes; i++)
	{
		outIndexes[i] = indexes[i];
	}

	BSPData::Vertice *outVertices = grid->vertices.data();

	ClearBounds(grid->cullInfo.bounds[0], grid->cullInfo.bounds[1]);

	for (int32_t i = 0; i < width; i++) {
		for (int32_t j = 0; j < height; j++) {
			vert = &outVertices[j*width + i];
			*vert = ctrl[j * MAX_GRID_SIZE + i];
			AddPointToBounds(vert->xyz, grid->cullInfo.bounds[0], grid->cullInfo.bounds[1]);
		}
	}

	// compute local origin and bounds
	castVector(grid->cullInfo.localOrigin) = castVector(grid->cullInfo.bounds[0]) + castVector(grid->cullInfo.bounds[1]);
	castVector(grid->cullInfo.localOrigin) *= 0.5f;
	tmpVec = castVector(grid->cullInfo.bounds[0]) - castVector(grid->cullInfo.localOrigin);
	grid->cullInfo.radius = (float)tmpVec.norm();
}

void BSPReader::SubdividePatchToGrid(int32_t Width, int32_t Height, const Vertice* Points, Surface* Out)
{
	int32_t i, j, k, l;
	Vertice prev;
	Vertice next;
	Vertice mid;
	float len, maxLen;
	int32_t dir;
	int32_t t;
	curveGrid_t* cgrid = new curveGrid_t;
	float errorTable[2][MAX_GRID_SIZE];
	int32_t numIndexes;
	int32_t indexes[(MAX_GRID_SIZE - 1)*(MAX_GRID_SIZE - 1) * 2 * 3];
	//int32_t consecutiveComplete;

	for (i = 0; i < Width; i++) {
		for (j = 0; j < Height; j++) {
			cgrid->ctrl[j][i] = Points[j*Width + i];
		}
	}

	for (dir = 0; dir < 2; dir++) {

		for (j = 0; j < MAX_GRID_SIZE; j++) {
			errorTable[dir][j] = 0;
		}

		//consecutiveComplete = 0;

		// horizontal subdivisions
		for (j = 0; j + 2 < Width; j += 2) {
			// check subdivided midpoints against control Points

			// FIXME: also check midpoints of adjacent patches against the control Points
			// this would basically stitch all patches in the same LOD group together.

			maxLen = 0;
			for (i = 0; i < Height; i++) {
				Vector3 midxyz;
				Vector3 midxyz2;
				Vector3 dir;
				Vector3 projected;
				float		d;

				// calculate the point on the curve
				for (l = 0; l < 3; l++) {
					midxyz[l] = (cgrid->ctrl[i][j].xyz[l] + cgrid->ctrl[i][j + 1].xyz[l] * 2
						+ cgrid->ctrl[i][j + 2].xyz[l]) * 0.25f;
				}

				// see how far off the line it is
				// using dist-from-line will not account for internal
				// texture warping, but it gives a lot less polygons than
				// dist-from-midpoint
				midxyz -= castVector(cgrid->ctrl[i][j].xyz);
				dir = castVector(cgrid->ctrl[i][j + 2].xyz) - castVector(cgrid->ctrl[i][j].xyz);
				dir.normalize();

				d = (float)midxyz.dot(dir);
				projected = dir * d;
				midxyz2 = midxyz - projected;
				len = (float)midxyz2.squaredNorm(); // we will do the sqrt later
				if (len > maxLen) {
					maxLen = len;
				}
			}

			maxLen = sqrt(maxLen);

			// if all the Points are on the lines, remove the entire columns
			if (maxLen < 0.1f) {
				errorTable[dir][j + 1] = 999;
				// if we go over the whole grid twice without adding any columns, stop
				//if (++consecutiveComplete >= Width)
				//	break;
				continue;
			}

			// see if we want to insert subdivided columns
			if (Width + 2 > MAX_GRID_SIZE) {
				errorTable[dir][j + 1] = 1.0f / maxLen;
				break;	// can't subdivide any more
			}

			if (maxLen <= MUM_SUBDIVISIONS) {
				errorTable[dir][j + 1] = 1.0f / maxLen;
				// if we go over the whole grid twice without adding any columns, stop
				//if (++consecutiveComplete >= Width)
				//	break;
				continue;	// didn't need subdivision
			}

			errorTable[dir][j + 2] = 1.0f / maxLen;

			//consecutiveComplete = 0;

			// insert two columns and replace the peak
			Width += 2;
			for (i = 0; i < Height; i++) {
				LerpDrawVert(&cgrid->ctrl[i][j], &cgrid->ctrl[i][j + 1], &prev);
				LerpDrawVert(&cgrid->ctrl[i][j + 1], &cgrid->ctrl[i][j + 2], &next);
				LerpDrawVert(&prev, &next, &mid);

				for (k = Width - 1; k > j + 3; k--) {
					cgrid->ctrl[i][k] = cgrid->ctrl[i][k - 2];
				}
				cgrid->ctrl[i][j + 1] = prev;
				cgrid->ctrl[i][j + 2] = mid;
				cgrid->ctrl[i][j + 3] = next;
			}

			// skip the new one, we'll get it on the next pass
			j += 2;
		}

		Transpose(Width, Height, cgrid->ctrl);
		t = Width;
		Width = Height;
		Height = t;
	}

	// put all the aproximating Points on the curve
	PutPointsOnCurve(cgrid->ctrl, Width, Height);

	// cull out any rows or columns that are colinear
	for (i = 1; i < Width - 1; i++) {
		if (errorTable[0][i] != 999) {
			continue;
		}
		for (j = i + 1; j < Width; j++) {
			for (k = 0; k < Height; k++) {
				cgrid->ctrl[k][j - 1] = cgrid->ctrl[k][j];
			}
			errorTable[0][j - 1] = errorTable[0][j];
		}
		Width--;
	}

	for (i = 1; i < Height - 1; i++) {
		if (errorTable[1][i] != 999) {
			continue;
		}
		for (j = i + 1; j < Height; j++) {
			for (k = 0; k < Width; k++) {
				cgrid->ctrl[j - 1][k] = cgrid->ctrl[j][k];
			}
			errorTable[1][j - 1] = errorTable[1][j];
		}
		Height--;
	}

	// flip for longest tristrips as an optimization
	// the results should be visually identical with or
	// without this step
	if (Height > Width) {
		Transpose(Width, Height, cgrid->ctrl);
		InvertErrorTable(errorTable, Width, Height);
		t = Width;
		Width = Height;
		Height = t;
		InvertCtrl(Width, Height, cgrid->ctrl);
	}

	// calculate indexes
	numIndexes = MakeMeshIndexes(Width, Height, indexes);

	// calculate normals
	MakeMeshNormals(Width, Height, cgrid->ctrl);

	CreateSurfaceGridMesh(Width, Height, reinterpret_cast<MOHPC::BSPData::Vertice*>(cgrid->ctrl), numIndexes, indexes, Out);

	delete cgrid;
}
