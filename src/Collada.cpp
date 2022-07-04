#include "Collada.hpp"

static std::string getCurrentIsoTime()
{
    std::stringstream sstream;
    std::time_t t = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    sstream << std::put_time(std::localtime(&t), "%FT%T%z");
    return sstream.str();
}

void buildInputShared(tinyxml2::XMLElement* parent, uint32_t offset, std::string semantic, std::string source)
{
    auto node = parent->InsertNewChildElement("input");
    node->SetAttribute("offset", offset);
    node->SetAttribute("semantic", semantic.c_str());
    node->SetAttribute("source", source.c_str());
}

void buildInputUnshared(tinyxml2::XMLElement* parent, std::string semantic, std::string source)
{
    auto node = parent->InsertNewChildElement("input");
    node->SetAttribute("semantic", semantic.c_str());
    node->SetAttribute("source", source.c_str());
}

void buildParamData(tinyxml2::XMLElement* parent,
                    std::string type,
                    std::optional<std::string> name     = {},
                    std::optional<std::string> semantic = {},
                    std::optional<std::string> sid      = {})
{
    auto node = parent->InsertNewChildElement("param");

    node->SetAttribute("type", type.c_str());

    if (name.has_value()) node->SetAttribute("name", name.value().c_str());
    if (semantic.has_value()) node->SetAttribute("semantic", semantic.value().c_str());
    if (sid.has_value()) node->SetAttribute("sid", sid.value().c_str());
}

template<typename T> static void buildSource(tinyxml2::XMLElement* meshNode, std::vector<T>& vector, std::string id)
{
    auto posSource = meshNode->InsertNewChildElement("source");
    posSource->SetAttribute("id", id.c_str());

    auto posArray = posSource->InsertNewChildElement("float_array");
    posArray->SetAttribute("id", (id + "-array").c_str());
    posArray->SetAttribute("count", vector.size() * 3);

    std::stringstream posString;
    for (const T& vec : vector)
        posString << vec << " ";

    posArray->SetText(posString.str().c_str());

    auto posTech     = posSource->InsertNewChildElement("technique_common");
    auto posAccessor = posTech->InsertNewChildElement("accessor");

    posAccessor->SetAttribute("count", vector.size());
    posAccessor->SetAttribute("source", ("#" + id + "-array").c_str());
    posAccessor->SetAttribute("stride", 3);

    buildParamData(posAccessor, "float", "X");
    buildParamData(posAccessor, "float", "Y");
    buildParamData(posAccessor, "float", "Z");
}

template<> static void buildSource(tinyxml2::XMLElement* meshNode, std::vector<Color>& vector, std::string id)
{
    auto posSource = meshNode->InsertNewChildElement("source");
    posSource->SetAttribute("id", id.c_str());

    auto posArray = posSource->InsertNewChildElement("float_array");
    posArray->SetAttribute("id", (id + "-array").c_str());
    posArray->SetAttribute("count", vector.size() * 3);

    std::stringstream posString;
    for (const Color& color : vector)
        posString << color << " ";

    posArray->SetText(posString.str().c_str());

    auto posTech     = posSource->InsertNewChildElement("technique_common");
    auto posAccessor = posTech->InsertNewChildElement("accessor");

    posAccessor->SetAttribute("count", vector.size());
    posAccessor->SetAttribute("source", ("#" + id + "-array").c_str());
    posAccessor->SetAttribute("stride", 3);

    buildParamData(posAccessor, "float", "R");
    buildParamData(posAccessor, "float", "G");
    buildParamData(posAccessor, "float", "B");
}

template<> static void buildSource(tinyxml2::XMLElement* meshNode, std::vector<TexCoord>& vector, std::string id)
{
    auto posSource = meshNode->InsertNewChildElement("source");
    posSource->SetAttribute("id", id.c_str());

    auto posArray = posSource->InsertNewChildElement("float_array");
    posArray->SetAttribute("id", (id + "-array").c_str());
    posArray->SetAttribute("count", vector.size() * 2);

    std::stringstream posString;
    for (const TexCoord& uv : vector)
        posString << uv << " ";

    posArray->SetText(posString.str().c_str());

    auto posTech     = posSource->InsertNewChildElement("technique_common");
    auto posAccessor = posTech->InsertNewChildElement("accessor");

    posAccessor->SetAttribute("count", vector.size());
    posAccessor->SetAttribute("source", ("#" + id + "-array").c_str());
    posAccessor->SetAttribute("stride", 2);

    buildParamData(posAccessor, "float", "S");
    buildParamData(posAccessor, "float", "T");
}

static void buildAnimSource(tinyxml2::XMLElement* meshNode,
                            std::vector<std::pair<float, float>>& vector,
                            std::string id,
                            uint32_t nodeId,
                            std::string axis,
                            std::string value)
{
    // time
    auto timeSource = meshNode->InsertNewChildElement("source");
    timeSource->SetAttribute("id", (id + "-time").c_str());

    auto timeArray = timeSource->InsertNewChildElement("float_array");
    timeArray->SetAttribute("id", (id + "-time-array").c_str());
    timeArray->SetAttribute("count", vector.size());

    // value
    auto valueSource = meshNode->InsertNewChildElement("source");
    valueSource->SetAttribute("id", (id + "-value").c_str());

    auto valueArray = valueSource->InsertNewChildElement("float_array");
    valueArray->SetAttribute("id", (id + "-value-array").c_str());
    valueArray->SetAttribute("count", vector.size());

    // interpolation
    auto interpolationSource = meshNode->InsertNewChildElement("source");
    interpolationSource->SetAttribute("id", (id + "-interpolation").c_str());

    auto interpolationArray = interpolationSource->InsertNewChildElement("Name_array");
    interpolationArray->SetAttribute("id", (id + "-interpolation-array").c_str());
    interpolationArray->SetAttribute("count", vector.size());

    std::stringstream timeString;
    std::stringstream valueString;
    std::stringstream interpolationString;
    for (const std::pair<float, float>& value : vector)
    {
        timeString << value.first << " ";
        valueString << value.second << " ";
        interpolationString << "LINEAR ";
    }

    timeArray->SetText(timeString.str().c_str());
    valueArray->SetText(valueString.str().c_str());
    interpolationArray->SetText(interpolationString.str().c_str());

    // time
    auto timeTech     = timeSource->InsertNewChildElement("technique_common");
    auto timeAccessor = timeTech->InsertNewChildElement("accessor");

    timeAccessor->SetAttribute("count", vector.size());
    timeAccessor->SetAttribute("source", ("#" + id + "-time-array").c_str());
    buildParamData(timeAccessor, "float", "TIME");

    // value
    auto valueTech     = valueSource->InsertNewChildElement("technique_common");
    auto valueAccessor = valueTech->InsertNewChildElement("accessor");

    valueAccessor->SetAttribute("count", vector.size());
    valueAccessor->SetAttribute("source", ("#" + id + "-value-array").c_str());
    buildParamData(valueAccessor, "float", value.c_str());

    // interpolation
    auto interpolationTech     = interpolationSource->InsertNewChildElement("technique_common");
    auto interpolationAccessor = interpolationTech->InsertNewChildElement("accessor");

    interpolationAccessor->SetAttribute("count", vector.size());
    interpolationAccessor->SetAttribute("source", ("#" + id + "-interpolation-array").c_str());
    buildParamData(interpolationAccessor, "Name", "INTERPOLATION");

    // sampler
    auto sampler = meshNode->InsertNewChildElement("sampler");
    sampler->SetAttribute("id", (id + "-sampler").c_str());
    buildInputUnshared(sampler, "INPUT", ("#" + id + "-time"));
    buildInputUnshared(sampler, "OUTPUT", ("#" + id + "-value"));
    buildInputUnshared(sampler, "INTERPOLATION", ("#" + id + "-interpolation"));

    // channel
    auto channel = meshNode->InsertNewChildElement("channel");
    channel->SetAttribute("source", ("#" + id + "-sampler").c_str());
    channel->SetAttribute("target", ("node-" + std::to_string(nodeId) + "/" + axis).c_str());
}

void ColladaExporter::setupXML()
{
    document.InsertEndChild(document.NewDeclaration());
    root = document.NewElement("COLLADA");
    root->SetAttribute("xmlns", "http://www.collada.org/2008/03/COLLADASchema");
    root->SetAttribute("version", "1.4.1");
    document.InsertEndChild(root);
}

void ColladaExporter::buildAssetNode()
{
    auto asset = root->InsertNewChildElement("asset");
    asset->InsertNewChildElement("created")->SetText(getCurrentIsoTime().c_str());
    asset->InsertNewChildElement("modified")->SetText(getCurrentIsoTime().c_str());
    asset->InsertNewChildElement("revision")->SetText("1.0.0");
    asset->InsertNewChildElement("up_axis")->SetText("Y_UP");
}

void ColladaExporter::addColorFaces(tinyxml2::XMLElement* node, Mesh& mesh, std::string id)
{
    std::vector<Color> colors;
    std::stringstream p;

    for (Face& face : mesh.faces)
    {
        if (face.material_type != MaterialType::COLOR) continue;

        p << face.v1 << " " << face.n1 << " " << colors.size() + 0 << " ";
        p << face.v2 << " " << face.n2 << " " << colors.size() + 1 << " ";
        p << face.v3 << " " << face.n3 << " " << colors.size() + 2 << " ";

        colors.push_back(face.color1);
        colors.push_back(face.color2);
        colors.push_back(face.color3);
    }

    if (colors.empty()) return;
    color = colors[0]; // used for material, only one color allowed

    buildSource(node, colors, id + "-color");

    // triangles element
    auto triNode = node->InsertNewChildElement("triangles");
    triNode->SetAttribute("count", colors.size() / 3);
    triNode->SetAttribute("material", "material-fixedColor");

    buildInputShared(triNode, 0, "VERTEX", ("#" + id + "-vertices"));
    buildInputShared(triNode, 1, "NORMAL", ("#" + id + "-normal"));
    buildInputShared(triNode, 2, "COLOR", ("#" + id + "-color"));

    triNode->InsertNewChildElement("p")->SetText(p.str().c_str());
}

void ColladaExporter::addTextureFaces(tinyxml2::XMLElement* node, Mesh& mesh, std::string id)
{
    std::vector<TexCoord> uvs;
    std::stringstream p;

    for (Face& face : mesh.faces)
    {
        if (face.material_type != MaterialType::TEXTURE) continue;

        p << face.v1 << " " << face.n1 << " " << uvs.size() + 0 << " ";
        p << face.v2 << " " << face.n2 << " " << uvs.size() + 1 << " ";
        p << face.v3 << " " << face.n3 << " " << uvs.size() + 2 << " ";

        uvs.push_back(TexCoord(face.uv1, tim.getSize()));
        uvs.push_back(TexCoord(face.uv2, tim.getSize()));
        uvs.push_back(TexCoord(face.uv3, tim.getSize()));
    }

    if (uvs.empty()) return;

    buildSource(node, uvs, id + "-texcoord");

    // triangles element
    auto triNode = node->InsertNewChildElement("triangles");
    triNode->SetAttribute("count", uvs.size() / 3);
    triNode->SetAttribute("material", "material-textured");

    buildInputShared(triNode, 0, "VERTEX", ("#" + id + "-vertices"));
    buildInputShared(triNode, 1, "NORMAL", ("#" + id + "-normal"));
    buildInputShared(triNode, 2, "TEXCOORD", ("#" + id + "-texcoord"));

    triNode->InsertNewChildElement("p")->SetText(p.str().c_str());
}

void ColladaExporter::addNoLightFaces(tinyxml2::XMLElement* node, Mesh& mesh, std::string id)
{
    std::vector<TexCoord> uvs;
    std::stringstream p;

    for (Face& face : mesh.faces)
    {
        if (face.material_type != MaterialType::NO_LIGHT) continue;

        p << face.v1 << " " << uvs.size() + 0 << " ";
        p << face.v2 << " " << uvs.size() + 1 << " ";
        p << face.v3 << " " << uvs.size() + 2 << " ";

        uvs.push_back(TexCoord(face.uv1, tim.getSize()));
        uvs.push_back(TexCoord(face.uv2, tim.getSize()));
        uvs.push_back(TexCoord(face.uv3, tim.getSize()));
    }

    if (uvs.empty()) return;

    buildSource(node, uvs, id + "-texcoord");

    // triangles element
    auto triNode = node->InsertNewChildElement("triangles");
    triNode->SetAttribute("count", uvs.size() / 3);
    triNode->SetAttribute("material", "material-rawTextured");

    buildInputShared(triNode, 0, "VERTEX", ("#" + id + "-vertices"));
    buildInputShared(triNode, 1, "TEXCOORD", ("#" + id + "-texcoord"));

    triNode->InsertNewChildElement("p")->SetText(p.str().c_str());
}

void ColladaExporter::buildGeometry()
{
    auto node = root->InsertNewChildElement("library_geometries");

    uint32_t i = 0;
    for (Mesh& mesh : model.meshes)
    {
        std::string id = "object-" + std::to_string(i++);
        auto geomNode  = node->InsertNewChildElement("geometry");
        geomNode->SetAttribute("id", id.c_str());

        auto meshNode = geomNode->InsertNewChildElement("mesh");
        buildSource(meshNode, mesh.vertices, id + "-pos");   // pos data
        buildSource(meshNode, mesh.normals, id + "-normal"); // normal data

        // vertices
        auto verticesNode = meshNode->InsertNewChildElement("vertices");
        verticesNode->SetAttribute("id", (id + "-vertices").c_str());
        buildInputUnshared(verticesNode, "POSITION", ("#" + id + "-pos"));

        // faces
        addColorFaces(meshNode, mesh, id);
        addTextureFaces(meshNode, mesh, id);
        addNoLightFaces(meshNode, mesh, id);
    }
}

void ColladaExporter::buildControllers()
{
    auto node = root->InsertNewChildElement("library_controllers");

    uint32_t i = 0;
    for (Mesh& mesh : model.meshes)
    {
        std::string id  = "object-" + std::to_string(i++);
        auto controller = node->InsertNewChildElement("controller");
        controller->SetAttribute("id", (id + "-skin").c_str());
        controller->SetAttribute("name", (id + "-armature").c_str());

        auto skin = controller->InsertNewChildElement("skin");
        skin->SetAttribute("source", ("#" + id).c_str());

        // joints source
        auto joints = skin->InsertNewChildElement("source");
        joints->SetAttribute("id", (id + "-joints").c_str());

        auto jointsArr = joints->InsertNewChildElement("Name_array");
        jointsArr->SetAttribute("count", 1);
        jointsArr->SetAttribute("id", (id + "-joints-array").c_str());

        std::string bla = "node-";
        bla += std::to_string(std::distance(model.skeleton.begin(),
                                            std::find_if(model.skeleton.begin(),
                                                         model.skeleton.end(),
                                                         [=](const NodeEntry node) { return node.object == i - 1; })));
        jointsArr->SetText(bla.c_str());

        // technique
        auto jointsTech       = joints->InsertNewChildElement("technique_common");
        auto jointsTechAccess = jointsTech->InsertNewChildElement("accessor");
        jointsTechAccess->SetAttribute("count", 1);
        jointsTechAccess->SetAttribute("stride", 1);
        jointsTechAccess->SetAttribute("source", ("#" + id + "-joints-array").c_str());
        buildParamData(jointsTechAccess, "Name", "JOINT");

        // poses source
        auto poses = skin->InsertNewChildElement("source");
        poses->SetAttribute("id", (id + "-poses").c_str());

        auto posesArr = poses->InsertNewChildElement("float_array");
        posesArr->SetAttribute("count", 16);
        posesArr->SetAttribute("id", (id + "-poses-array").c_str());
        posesArr->SetText("1 0 0 0  0 1 0 0  0 0 1 0  0 0 0 1");

        auto posesTech       = poses->InsertNewChildElement("technique_common");
        auto posesTechAccess = posesTech->InsertNewChildElement("accessor");
        posesTechAccess->SetAttribute("count", 1);
        posesTechAccess->SetAttribute("stride", 16);
        posesTechAccess->SetAttribute("source", ("#" + id + "-poses-array").c_str());
        buildParamData(posesTechAccess, "float4x4", "TRANSFORM");

        // weights source
        auto weights = skin->InsertNewChildElement("source");
        weights->SetAttribute("id", (id + "-weights").c_str());

        auto weightArr = weights->InsertNewChildElement("float_array");
        weightArr->SetAttribute("count", 1);
        weightArr->SetAttribute("id", (id + "-weights-array").c_str());
        weightArr->SetText("1");

        auto weightsTech = weights->InsertNewChildElement("technique_common");

        auto weightsTechAccess = weightsTech->InsertNewChildElement("accessor");
        weightsTechAccess->SetAttribute("count", 1);
        weightsTechAccess->SetAttribute("stride", 1);
        weightsTechAccess->SetAttribute("source", ("#" + id + "-weights-array").c_str());
        buildParamData(weightsTechAccess, "float", "WEIGHT");

        // joints
        auto j = skin->InsertNewChildElement("joints");
        buildInputUnshared(j, "JOINT", ("#" + id + "-joints"));
        buildInputUnshared(j, "INV_BIND_MATRIX", ("#" + id + "-poses"));

        // vertex weights
        auto vertexWeights = skin->InsertNewChildElement("vertex_weights");
        vertexWeights->SetAttribute("count", mesh.vertices.size());
        buildInputShared(vertexWeights, 0, "JOINT", ("#" + id + "-joints"));
        buildInputShared(vertexWeights, 1, "WEIGHT", ("#" + id + "-weights"));

        auto v      = vertexWeights->InsertNewChildElement("v");
        auto vcount = vertexWeights->InsertNewChildElement("vcount");

        std::stringstream vString;
        std::stringstream vcountString;

        for (uint32_t i = 0; i < mesh.vertices.size() * 2; i++)
            vString << "0 ";
        for (uint32_t i = 0; i < mesh.vertices.size(); i++)
            vcountString << "1 ";

        vcount->SetText(vcountString.str().c_str());
        v->SetText(vString.str().c_str());
    }
}

void ColladaExporter::buildScene()
{
    auto libraryScenes = root->InsertNewChildElement("library_visual_scenes");
    auto scene         = libraryScenes->InsertNewChildElement("visual_scene");
    auto rootNode      = scene->InsertNewChildElement("node");

    scene->SetAttribute("id", "scene");
    scene->SetAttribute("name", "scene");


    rootNode->SetAttribute("id", model.name.c_str());
    rootNode->SetAttribute("name", model.name.c_str());

    // mesh nodes
    auto meshNode = rootNode->InsertNewChildElement("node");
    meshNode->SetAttribute("id", "mesh");

    for (uint32_t i = 0; i < model.meshes.size(); i++)
    {
        auto subNode = meshNode->InsertNewChildElement("node");
        subNode->SetAttribute("id", ("object-" + std::to_string(i)).c_str());
        subNode->SetAttribute("name", ("object-" + std::to_string(i)).c_str());

        auto instance = subNode->InsertNewChildElement("instance_controller");
        instance->SetAttribute("url", ("#object-" + std::to_string(i) + "-skin").c_str());
        instance->InsertNewChildElement("skeleton")->SetText("#node-0");

        auto bindMaterial = instance->InsertNewChildElement("bind_material");
        auto tech = bindMaterial->InsertNewChildElement("technique_common");

        auto instanceTextured = tech->InsertNewChildElement("instance_material");
        instanceTextured->SetAttribute("symbol", "material-textured");
        instanceTextured->SetAttribute("target", "#mat-textured");
        auto uvInputTextured = instanceTextured->InsertNewChildElement("bind_vertex_input");
        uvInputTextured->SetAttribute("semantic", "UVSET0");
        uvInputTextured->SetAttribute("input_semantic", "TEXCOORD");

        auto instanceRawTextured = tech->InsertNewChildElement("instance_material");
        instanceRawTextured->SetAttribute("symbol", "material-rawTextured");
        instanceRawTextured->SetAttribute("target", "#mat-rawTextured");
        auto uvInputRawTextured = instanceRawTextured->InsertNewChildElement("bind_vertex_input");
        uvInputRawTextured->SetAttribute("semantic", "UVSET0");
        uvInputRawTextured->SetAttribute("input_semantic", "TEXCOORD");

        auto instanceFixedColor = tech->InsertNewChildElement("instance_material");
        instanceFixedColor->SetAttribute("symbol", "material-fixedColor");
        instanceFixedColor->SetAttribute("target", "#mat-fixedColor");
    }

    // joint nodes
    auto jointsNode = rootNode->InsertNewChildElement("node");
    jointsNode->SetAttribute("id", "joints");

    std::vector<tinyxml2::XMLElement*> xmlNodes;

    for (uint32_t i = 0; i < model.skeleton.size() || i == 0; i++)
    {
        NodeEntry entry{ 0xFF, 0xFF };
        if (model.skeleton.size() != 0) entry = model.skeleton[i];
        auto parent = entry.parent == 0xFF ? jointsNode : xmlNodes[entry.parent];

        auto node = parent->InsertNewChildElement("node");
        node->SetAttribute("id", ("node-" + std::to_string(i)).c_str());
        node->SetAttribute("name", ("node-" + std::to_string(i)).c_str());
        node->SetAttribute("sid", ("node-" + std::to_string(i)).c_str());
        node->SetAttribute("type", "JOINT");

        auto scale = node->InsertNewChildElement("scale");
        scale->SetAttribute("sid", "scale");
        scale->SetText("1 1 1");
        auto translate = node->InsertNewChildElement("translate");
        translate->SetAttribute("sid", "translate");
        translate->SetText("0 0 0");

        auto rotX = node->InsertNewChildElement("rotate");
        rotX->SetAttribute("sid", "rotateX");
        rotX->SetText("1 0 0 0");

        auto rotY = node->InsertNewChildElement("rotate");
        rotY->SetAttribute("sid", "rotateY");
        rotY->SetText("0 1 0 0");

        auto rotZ = node->InsertNewChildElement("rotate");
        rotZ->SetAttribute("sid", "rotateZ");
        rotZ->SetText("0 0 1 0");

        xmlNodes.push_back(node);
    }

    auto bla = root->InsertNewChildElement("scene");
    bla->InsertNewChildElement("instance_visual_scene")->SetAttribute("url", "#scene");
}

void ColladaExporter::buildAnimations()
{
    auto libAnims = root->InsertNewChildElement("library_animations");
    auto libClips = root->InsertNewChildElement("library_animation_clips");
    uint32_t id   = 0;

    for (auto& a : model.anims->anims)
    {
        std::string animId = "anim-" + std::to_string(id);
        const Animation animData(a);

        auto animNode = libAnims->InsertNewChildElement("animation");
        animNode->SetAttribute("name", animId.c_str());
        animNode->SetAttribute("id", animId.c_str());

        uint32_t nodeId = 0;

        for (auto data : animData.getData())
        {
            std::string sourceId = animId + "-node" + std::to_string(nodeId);

            buildAnimSource(animNode, data.data[Axis::SCALE_X], (sourceId + "-scaleX").c_str(), nodeId, "scale.X", "X");
            buildAnimSource(animNode, data.data[Axis::SCALE_Y], (sourceId + "-scaleY").c_str(), nodeId, "scale.Y", "Y");
            buildAnimSource(animNode, data.data[Axis::SCALE_Z], (sourceId + "-scaleZ").c_str(), nodeId, "scale.Z", "Z");
            buildAnimSource(animNode,
                            data.data[Axis::ROT_X],
                            (sourceId + "-rotX").c_str(),
                            nodeId,
                            "rotateX.ANGLE",
                            "ANGLE");
            buildAnimSource(animNode,
                            data.data[Axis::ROT_Y],
                            (sourceId + "-rotY").c_str(),
                            nodeId,
                            "rotateY.ANGLE",
                            "ANGLE");
            buildAnimSource(animNode,
                            data.data[Axis::ROT_Z],
                            (sourceId + "-rotZ").c_str(),
                            nodeId,
                            "rotateZ.ANGLE",
                            "ANGLE");
            buildAnimSource(animNode, data.data[Axis::POS_X], (sourceId + "-posX").c_str(), nodeId, "translate.X", "X");
            buildAnimSource(animNode, data.data[Axis::POS_Y], (sourceId + "-posY").c_str(), nodeId, "translate.Y", "Y");
            buildAnimSource(animNode, data.data[Axis::POS_Z], (sourceId + "-posZ").c_str(), nodeId, "translate.Z", "Z");
            nodeId++;
        }

        auto clipNode = libClips->InsertNewChildElement("animation_clip");
        clipNode->SetAttribute("name", ("animation" + std::to_string(id)).c_str());
        clipNode->SetAttribute("start", 0.0);
        clipNode->SetAttribute("end", a.frameCount * 0.05);
        clipNode->InsertNewChildElement("instance_animation")->SetAttribute("url", ("#" + animId).c_str());
        id++;
    }
}

void ColladaExporter::buildImages()
{
    auto libImages = root->InsertNewChildElement("library_images");

    auto image = libImages->InsertNewChildElement("image");
    image->SetAttribute("id", "texture");
    image->SetAttribute("name", "texture");
    auto initFrom = image->InsertNewChildElement("init_from");

    initFrom->SetText(model.texture->string().c_str());
}

void ColladaExporter::buildEffect1(tinyxml2::XMLElement* libEffects)
{
    auto effect = libEffects->InsertNewChildElement("effect");
    effect->SetAttribute("id", "fx-textured");
    auto profile  = effect->InsertNewChildElement("profile_COMMON");

    auto texParam = profile->InsertNewChildElement("newparam");
    texParam->SetAttribute("sid", "tex");
    auto surface = texParam->InsertNewChildElement("surface");
    surface->SetAttribute("type", "2D");
    surface->InsertNewChildElement("init_from")->SetText("texture");
    
    auto samplerParam = profile->InsertNewChildElement("newparam");
    samplerParam->SetAttribute("sid", "texture-sampler");
    auto sampler = samplerParam->InsertNewChildElement("sampler2D");
    sampler->InsertNewChildElement("source")->SetText("tex");

    auto technique = profile->InsertNewChildElement("technique");
    technique->SetAttribute("sid", "COMMON");

    auto shader         = technique->InsertNewChildElement("lambert");
    auto diffuse        = shader->InsertNewChildElement("diffuse");
    auto diffuseTexture = diffuse->InsertNewChildElement("texture");
    diffuseTexture->SetAttribute("texcoord", "UVSET0");
    diffuseTexture->SetAttribute("texture", "texture-sampler");
}

void ColladaExporter::buildEffect2(tinyxml2::XMLElement* libEffects)
{
    auto effect = libEffects->InsertNewChildElement("effect");
    effect->SetAttribute("id", "fx-rawTextured");
    auto profile  = effect->InsertNewChildElement("profile_COMMON");

    auto texParam = profile->InsertNewChildElement("newparam");
    texParam->SetAttribute("sid", "tex");
    auto surface = texParam->InsertNewChildElement("surface");
    surface->SetAttribute("type", "2D");
    surface->InsertNewChildElement("init_from")->SetText("texture");
    
    auto samplerParam = profile->InsertNewChildElement("newparam");
    samplerParam->SetAttribute("sid", "texture-sampler");
    auto sampler = samplerParam->InsertNewChildElement("sampler2D");
    sampler->InsertNewChildElement("source")->SetText("tex");

    auto technique = profile->InsertNewChildElement("technique");
    technique->SetAttribute("sid", "COMMON");

    auto shader         = technique->InsertNewChildElement("lambert");
    auto diffuse        = shader->InsertNewChildElement("diffuse");
    auto diffuseTexture = diffuse->InsertNewChildElement("texture");
    diffuseTexture->SetAttribute("texcoord", "UVSET0");
    diffuseTexture->SetAttribute("texture", "texture-sampler");
}

void ColladaExporter::buildEffect3(tinyxml2::XMLElement* libEffects)
{
    auto effect = libEffects->InsertNewChildElement("effect");
    effect->SetAttribute("id", "fx-fixedColor");
    auto profile   = effect->InsertNewChildElement("profile_COMMON");
    auto technique = profile->InsertNewChildElement("technique");
    technique->SetAttribute("sid", "COMMON");

    auto shader  = technique->InsertNewChildElement("lambert");
    auto diffuse = shader->InsertNewChildElement("diffuse");
    diffuse->InsertNewChildElement("color")->SetText(
        (std::to_string(color.r / 255.0f) + " " + std::to_string(color.g / 255.0f) + " " + std::to_string(color.b / 255.0f) + " 1").c_str());
}

void ColladaExporter::buildEffects()
{
    auto libEffects = root->InsertNewChildElement("library_effects");
    buildEffect1(libEffects);
    buildEffect2(libEffects);
    buildEffect3(libEffects);
}

void ColladaExporter::buildMaterials() { 
    auto libMaterials = root->InsertNewChildElement("library_materials");

    auto matTexture = libMaterials->InsertNewChildElement("material");
    matTexture->SetAttribute("id", "mat-textured");
    matTexture->SetAttribute("name", "mat-textured");
    matTexture->InsertNewChildElement("instance_effect")->SetAttribute("url", "#fx-textured");

    
    auto matRawTexture = libMaterials->InsertNewChildElement("material");
    matRawTexture->SetAttribute("id", "mat-rawTextured");
    matRawTexture->SetAttribute("name", "mat-rawTextured");
    matRawTexture->InsertNewChildElement("instance_effect")->SetAttribute("url", "#fx-rawTextured");

    
    auto matFixedColor = libMaterials->InsertNewChildElement("material");
    matFixedColor->SetAttribute("id", "mat-fixedColor");
    matFixedColor->SetAttribute("name", "mat-fixedColor");
    matFixedColor->InsertNewChildElement("instance_effect")->SetAttribute("url", "#fx-fixedColor");
}

void ColladaExporter::buildXML()
{
    buildAssetNode();

    buildGeometry();
    buildControllers();
    buildAnimations();

    buildImages();
    buildEffects();
    buildMaterials();

    buildScene();
}

ColladaExporter::ColladaExporter(Model& model, AbstractTIM& tim)
    : model(model)
    , tim(tim)
{
    setupXML();
    buildXML();
}

tinyxml2::XMLDocument& ColladaExporter::getDocument() { return document; }