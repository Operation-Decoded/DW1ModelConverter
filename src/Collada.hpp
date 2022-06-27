#pragma once

#include "Model.hpp"
#include "TIM.hpp"
#include "tinyxml2/tinyxml2.h"

class ColladaExporter
{
private:
    tinyxml2::XMLDocument document;
    tinyxml2::XMLElement* root;

    Model& model;
    AbstractTIM& tim;

private:
    void setupXML();
    void buildAssetNode();
    void addColorFaces(tinyxml2::XMLElement* node, Mesh& mesh, std::string id);
    void addTextureFaces(tinyxml2::XMLElement* node, Mesh& mesh, std::string id);

    void buildGeometry();
    void buildControllers();
    void buildXML();
    void buildScene();
    void buildAnimations();

public:
    ColladaExporter(Model& model, AbstractTIM& tim);
    tinyxml2::XMLDocument& getDocument();
};