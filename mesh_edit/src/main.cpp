#include "CGL/CGL.h"

#include "collada.h"
#include "meshEdit.h" //contains octree.h
#include "bezierPatch.h"
#include "bezierCurve.h"
#include "mergeVertices.h"
#include "shaderUtils.h"
#include "pointcloud.h" //includes marchingcubes.h which includes octree.h :/

#include <iostream>

using namespace std;
using namespace CGL;

#define msg(s) cerr << "[Collada Viewer] " << s << endl;

int loadFile(MeshEdit* collada_viewer, const char* path) {

  //made a MeshEdit local Octree var and in this function, set collada_viewer's Octree local var, then when we press V after the
  //file has been loaded, then we should see the Octree since the visualize function will
  //be able to work from within the


  Scene* scene = new Scene();

  std::string path_str = path;
  if (path_str.substr(path_str.length()-4, 4) == ".dae")
  {
    if (ColladaParser::load(path, scene) < 0) {
      delete scene;
      return -1;
    }
  }
  else if (path_str.substr(path_str.length()-4, 4) == ".bez")
  {
    Camera* cam = new Camera();
    cam->type = CAMERA;
    Node node;
    node.instance = cam;
    scene->nodes.push_back(node);
    Polymesh* mesh = new Polymesh();

    FILE* file = fopen(path, "r");
    int n = 0;
    fscanf(file, "%d", &n);
    for (int i = 0; i < n; i++)
    {
      BezierPatch patch;
      patch.loadControlPoints(file);
      patch.add2mesh(mesh);
      mergeVertices(mesh);
    }
    fclose(file);

    mesh->type = POLYMESH;
    node.instance = mesh;
    scene->nodes.push_back(node);
  }
  else if (path_str.substr(path_str.length()-4, 4) == ".txt")
  {
    Camera* cam = new Camera();
    cam->type = CAMERA;
    Node node;
    node.instance = cam;
    scene->nodes.push_back(node);
    Polymesh* mesh = new Polymesh();

    FILE* file = fopen(path, "r");
    int n = 0;
    fscanf(file, "%d", &n);

    PointCloud pc(n);
    pc.loadPoints(file);
    pc.add2mesh(mesh);
    //mergeVertices(mesh);
    fclose(file);

    mesh->type = POLYMESH;
    node.instance = mesh;
    scene->nodes.push_back(node);
  }
  else if (path_str.substr(path_str.length()-4, 4) == ".rtf")
  {
    Camera* cam = new Camera();
    cam->type = CAMERA;
    Node node;
    node.instance = cam;
    scene->nodes.push_back(node);
    Polymesh* mesh = new Polymesh();

    FILE* file = fopen(path, "r");
    int n = 0;
    //fscanf(file, "%d", &n);

    //pass in a BBOX that is constructed based on the sphere being made
    BBox bb(Vector3D(-10, -10, -10), Vector3D(10, 10, 10)); //MAKE BBOX FOR SPHERE TEST
    OctreeNode oct(nullptr, 0, bb, 3); //CONSTRUCT FULL OCTREE
    collada_viewer->oNode = &oct;
    //maybe to test BBOX, don't run the marching cubes?


    PointCloud pc(10); //make pointcloud for mesh constructin
    Mesh* mResult = new Mesh;
    marchingCubes(oct, IndicatorFunction, mResult);
    pc.loadMesh(mesh, *mResult);


    //mergeVertices(mesh);
    fclose(file);

    mesh->type = POLYMESH;
    node.instance = mesh;
    scene->nodes.push_back(node);
  }

  collada_viewer->load( scene );

  GLuint tex = makeTex("envmap/envmap.png");
  if(!tex) tex = makeTex("../envmap/envmap.png");
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, tex);
  glActiveTexture(GL_TEXTURE2);

  return 0;
}

int main( int argc, char** argv ) {

  const char* path = argv[1];
  std::string path_str = path;

  //////////////////////////////
  // Bezier curve viewer code //
  //////////////////////////////

  if (path_str.substr(path_str.length()-4, 4) == ".bzc")
  {
    // Each file contains a single Bezier curve's control points
    FILE* file = fopen(path, "r");

    int numControlPoints;
    fscanf(file, "%d", &numControlPoints);

    BezierCurve curve(numControlPoints);
    curve.loadControlPoints(file);
    fclose(file);

    // Create viewer
    Viewer viewer = Viewer();
    viewer.set_renderer(&curve);
    viewer.init();
    viewer.start();

    exit(EXIT_SUCCESS);

    return 0;
  }

  // create viewer
  Viewer viewer = Viewer();

  // create collada_viewer
  MeshEdit* collada_viewer = new MeshEdit();

  // set collada_viewer as renderer
  viewer.set_renderer(collada_viewer);

  // init viewer
  viewer.init();

  // load tests
  if( argc == 2 ) {
    if (loadFile(collada_viewer, argv[1]) < 0) exit(0);
  } else {
    msg("Usage: ./meshedit <path to scene file>"); exit(0);
  }

  // start viewer
  viewer.start();

  return 0;
}
