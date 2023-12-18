//#define _WITH_TEXTURE

using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

using System.IO;
using UnityEditor;
using System.Text;

public class BinarySceneWriterWithMaterial : MonoBehaviour
{
    private List<string> m_strGameObjectNames = new List<string>();
    private List<string> m_strTextureNames = new List<string>();
    private List<string> m_strMeshNames = new List<string>();
    private List<string> m_strMaterialNames = new List<string>();

    void WriteMatrix(BinaryWriter binaryWriter, Matrix4x4 matrix)
    {
        binaryWriter.Write(matrix.m00);
        binaryWriter.Write(matrix.m10);
        binaryWriter.Write(matrix.m20);
        binaryWriter.Write(matrix.m30);
        binaryWriter.Write(matrix.m01);
        binaryWriter.Write(matrix.m11);
        binaryWriter.Write(matrix.m21);
        binaryWriter.Write(matrix.m31);
        binaryWriter.Write(matrix.m02);
        binaryWriter.Write(matrix.m12);
        binaryWriter.Write(matrix.m22);
        binaryWriter.Write(matrix.m32);
        binaryWriter.Write(matrix.m03);
        binaryWriter.Write(matrix.m13);
        binaryWriter.Write(matrix.m23);
        binaryWriter.Write(matrix.m33);
    }

    void WriteLocalMatrix(BinaryWriter binaryWriter, Transform transform)
    {
        Matrix4x4 matrix = Matrix4x4.identity;
        matrix.SetTRS(transform.localPosition, transform.localRotation, transform.localScale);
        WriteMatrix(binaryWriter, matrix);
    }

    void WriteWorldMatrix(BinaryWriter binaryWriter, Transform transform)
    {
        Matrix4x4 matrix = Matrix4x4.identity;
        matrix.SetTRS(transform.position, transform.rotation, transform.lossyScale);
        WriteMatrix(binaryWriter, matrix);
    }

    void WriteFloat(BinaryWriter binaryWriter, string strHeader, float f)
    {
        binaryWriter.Write(strHeader);
        binaryWriter.Write(f);
    }

    void WriteColor(BinaryWriter binaryWriter, Color color)
    {
        binaryWriter.Write(color.r);
        binaryWriter.Write(color.g);
        binaryWriter.Write(color.b);
        binaryWriter.Write(color.a);
    }

    void WriteColor(BinaryWriter binaryWriter, string strHeader, Color color)
    {
        binaryWriter.Write(strHeader);
        WriteColor(binaryWriter, color);
    }

    void WriteObjectName(BinaryWriter binaryWriter, string strHeader, string strName)
    {
        binaryWriter.Write(strHeader);
        string strObjectName = string.Copy(strName).Replace(" ", "_");
        binaryWriter.Write(strObjectName);
    }

    void Write2DVector(BinaryWriter binaryWriter, Vector2 v)
    {
        binaryWriter.Write(v.x);
        binaryWriter.Write(v.y);
    }

    void Write2DVectors(BinaryWriter binaryWriter, string strName, Vector2[] pVectors)
    {
        binaryWriter.Write(strName);
        binaryWriter.Write(pVectors.Length);
        foreach (Vector2 v in pVectors) Write2DVector(binaryWriter, v);
    }

    void Write3DVector(BinaryWriter binaryWriter, Vector3 v)
    {
        binaryWriter.Write(v.x);
        binaryWriter.Write(v.y);
        binaryWriter.Write(v.z);
    }

    void Write3DVectors(BinaryWriter binaryWriter, string strName, Vector3[] pVectors)
    {
        binaryWriter.Write(strName);
        binaryWriter.Write(pVectors.Length);
        foreach (Vector3 v in pVectors) Write3DVector(binaryWriter, v);
    }

    void WriteIntegers(BinaryWriter binaryWriter, int[] pIntegers)
    {
        binaryWriter.Write(pIntegers.Length);
        foreach (int i in pIntegers) binaryWriter.Write(i);
    }

    void WriteIntegers(BinaryWriter binaryWriter, string strName, int[] pIntegers)
    {
        binaryWriter.Write(strName);
        binaryWriter.Write(pIntegers.Length);
        foreach (int i in pIntegers) binaryWriter.Write(i);
    }

    void WriteBoundingBox(BinaryWriter binaryWriter, string strName, Bounds bounds)
    {
        binaryWriter.Write(strName);
        Write3DVector(binaryWriter, bounds.center);
        Write3DVector(binaryWriter, bounds.extents);
    }

    void WriteSubMeshes(BinaryWriter binaryWriter, string strName, Mesh mesh)
    {
        binaryWriter.Write(strName);
        int nSubMeshes = mesh.subMeshCount;
        binaryWriter.Write(nSubMeshes);
        for (int i = 0; i < nSubMeshes; i++)
        {
            uint nSubMeshStart = mesh.GetIndexStart(i);
            uint nSubMeshIndices = mesh.GetIndexCount(i);
            int[] pSubMeshIndices = mesh.GetIndices(i);
            binaryWriter.Write(nSubMeshStart);
            binaryWriter.Write(nSubMeshIndices);
            WriteIntegers(binaryWriter, pSubMeshIndices);
        }
    }

    void WriteMaterials(BinaryWriter binaryWriter, Material[] materials)
    {
        binaryWriter.Write("<Materials>:");
        binaryWriter.Write(materials.Length);

        for (int i = 0; i < materials.Length; i++)
        {
            string strFilteredMaterialName = GetFilteredName(materials[i].name);
            bool bDuplicatedMaterial = FindMaterialByName(strFilteredMaterialName);
            WriteObjectName(binaryWriter, "<Material>:", (bDuplicatedMaterial) ? "@" + strFilteredMaterialName : "#" + strFilteredMaterialName);
            print("<Material>: " + ((bDuplicatedMaterial) ? "@" + strFilteredMaterialName : "#" + strFilteredMaterialName));

            if (!bDuplicatedMaterial)
            {
                WriteColor(binaryWriter, "<AlbedoColor>:", (materials[i].HasProperty("_Color")) ? materials[i].GetColor("_Color") : Color.black);
                WriteColor(binaryWriter, "<EmissiveColor>:", (materials[i].HasProperty("_EmissionColor")) ? materials[i].GetColor("_EmissionColor") : Color.black);
                WriteColor(binaryWriter, "<SpecularColor>:", (materials[i].HasProperty("_SpecColor")) ? materials[i].GetColor("_SpecColor") : Color.black);
                WriteFloat(binaryWriter, "<Glossiness>:", (materials[i].HasProperty("_Glossiness")) ? materials[i].GetFloat("_Glossiness") : 0.0f);
                WriteFloat(binaryWriter, "<Smoothness>:", (materials[i].HasProperty("_Smoothness")) ? materials[i].GetFloat("_Smoothness") : 0.0f);
                WriteFloat(binaryWriter, "<Metallic>:", (materials[i].HasProperty("_Metallic")) ? materials[i].GetFloat("_Metallic") : 0.0f);
                WriteFloat(binaryWriter, "<SpecularHighlight>:", (materials[i].HasProperty("_SpecularHighlights")) ? materials[i].GetFloat("_SpecularHighlights") : 0.0f);
                WriteFloat(binaryWriter, "<GlossyReflection>:", (materials[i].HasProperty("_GlossyReflections")) ? materials[i].GetFloat("_GlossyReflections") : 0.0f);
#if (_WITH_TEXTURE)
                Texture albedoTexture = materials[i].GetTexture("_MainTex");
                if (albedoTexture)
                {
                    binaryWriter.Write("<AlbedoTextureName>:");
                    binaryWriter.Write(string.Copy(albedoTexture.name).Replace(" ", "_"));
                }
                else
                {
                    binaryWriter.Write("<Null>:");
                }
#endif
#if (_WITH_TEXTURE)
                Texture emissionTexture = materials[i].GetTexture("_EmissionMap");
                if (emissionTexture)
                {
                    binaryWriter.Write("<EmissionTextureName>:");
                    binaryWriter.Write(string.Copy(emissionTexture.name).Replace(" ", "_"));
                }
                else
                {
                    binaryWriter.Write("<Null>:");
                }
#endif
            }

            binaryWriter.Write("</Material>");
        }

        binaryWriter.Write("</Materials>");
    }

    void WriteMesh(BinaryWriter binaryWriter, Mesh mesh, string strObjectName)
    {
        bool bDuplicatedMesh = FindMeshByName(strObjectName);
        WriteObjectName(binaryWriter, "<Mesh>:", (bDuplicatedMesh) ? "@" + strObjectName : "#" + strObjectName);
        print("<Mesh>: " + ((bDuplicatedMesh) ? "@" + strObjectName : "#" + strObjectName));
        //        bool bDuplicatedMesh = FindMeshByName(mesh.name);
        //        WriteObjectName(binaryWriter, "<Mesh>:", (bDuplicatedMesh) ? "@" + mesh.name : "#" + mesh.name);

        if (!bDuplicatedMesh)
        {
            BinaryWriter binaryMeshWriter = new BinaryWriter(File.Open(strObjectName + ".bin", FileMode.Create));

            WriteBoundingBox(binaryMeshWriter, "<BoundingBox>:", mesh.bounds); //AABB

            Write3DVectors(binaryMeshWriter, "<Positions>:", mesh.vertices);
            Write3DVectors(binaryMeshWriter, "<Normals>:", mesh.normals);
#if _WITH_TEXTURE
        Write2DVectors(binaryMeshWriter, "<TextureCoords>:", mesh.uv);
#endif
            WriteIntegers(binaryMeshWriter, "<Indices>:", mesh.triangles);

            WriteSubMeshes(binaryMeshWriter, "<SubMeshes>:", mesh);

            binaryMeshWriter.Flush();
            binaryMeshWriter.Close();
        }

        binaryWriter.Write("</Mesh>");
    }

    bool FindMeshByName(string strMeshName)
    {
        for (int i = 0; i < m_strMeshNames.Count; i++)
        {
            if (m_strMeshNames.Contains(strMeshName)) return (true);
        }
        m_strMeshNames.Add(strMeshName);
        return (false);
    }

    bool FindTextureByName(string strTextureName)
    {
        for (int i = 0; i < m_strTextureNames.Count; i++)
        {
            if (m_strTextureNames.Contains(strTextureName)) return (true);
        }
        m_strTextureNames.Add(strTextureName);
        return (false);
    }

    bool FindObjectByName(string strObjectName)
    {
        for (int i = 0; i < m_strGameObjectNames.Count; i++)
        {
            if (m_strGameObjectNames.Contains(strObjectName)) return (true);
        }
        m_strGameObjectNames.Add(strObjectName);
        return (false);
    }

    bool FindMaterialByName(string strMaterialName)
    {
        for (int i = 0; i < m_strMaterialNames.Count; i++)
        {
            if (m_strMaterialNames.Contains(strMaterialName)) return (true);
        }
        m_strMaterialNames.Add(strMaterialName);
        return (false);
    }

    string GetFilteredName(string name)
    {
        string strToRemove = " (";
        int i = name.IndexOf(strToRemove);
        string strNewName = (i > 0) ? string.Copy(name).Remove(i) : string.Copy(name);
        string strFilteredName = strNewName.Replace(' ', '_');
        return (strFilteredName);
    }

    void WriteFrameInfo(BinaryWriter binaryWriter, Transform transform)
    {
        string strFilteredObjectName = GetFilteredName(transform.name);

        WriteObjectName(binaryWriter, "<GameObject>:", strFilteredObjectName);
        print("<GameObject>: " + strFilteredObjectName);

        WriteWorldMatrix(binaryWriter, transform);

        MeshFilter meshFilter = transform.gameObject.GetComponent<MeshFilter>();
        MeshRenderer meshRenderer = transform.gameObject.GetComponent<MeshRenderer>();

        if (meshFilter && meshRenderer)
        {
            WriteMesh(binaryWriter, meshFilter.mesh, strFilteredObjectName);
            WriteMaterials(binaryWriter, meshRenderer.materials);
        }

        binaryWriter.Write("</GameObject>");
    }

    void WriteFrameHierarchy(BinaryWriter binaryWriter, Transform transform)
    {
        WriteFrameInfo(binaryWriter, transform);

        for (int k = 0; k < transform.childCount; k++)
        {
            WriteFrameHierarchy(binaryWriter, transform.GetChild(k));
        }
    }

    int GetChildObjectsCount(Transform transform)
    {
        int nChilds = 1;

        for (int k = 0; k < transform.childCount; k++) nChilds += GetChildObjectsCount(transform.GetChild(k));

        return (nChilds);
    }

    int GetAllGameObjectsCount()
    {
        int nGameObjects = 0;

        Scene scene = transform.gameObject.scene;
        GameObject[] rootGameObjects = scene.GetRootGameObjects();
        foreach (GameObject gameObject in rootGameObjects) nGameObjects += GetChildObjectsCount(gameObject.transform);

        return (nGameObjects);
    }

    int GetTexturesCount(Material[] materials)
    {
        int nTextures = 0;
        for (int i = 0; i < materials.Length; i++)
        {
            Texture albedoTexture = materials[i].GetTexture("_MainTex"); //materials[i].mainTexture
            if (albedoTexture) nTextures++;
            Texture emissionTexture = materials[i].GetTexture("_EmissionMap");
            if (albedoTexture) nTextures++;
        }
        return (nTextures);
    }

    int GetTexturesCount(Transform transform)
    {
        int nTextures = 0;
        string strObjectName = string.Copy(transform.name).Replace(" ", "_");
        if (FindTextureByName(strObjectName) == false)
        {
            MeshRenderer meshRenderer = transform.gameObject.GetComponent<MeshRenderer>();
            if (meshRenderer) nTextures = GetTexturesCount(meshRenderer.materials);
        }
        for (int k = 0; k < transform.childCount; k++) nTextures += GetTexturesCount(transform.GetChild(k));
        return (nTextures);
    }

    void CollectMeshesAndMaterials(BinaryWriter binaryWriter, Transform transform)
    {
        string strFilteredObjectName = GetFilteredName(transform.name);

        MeshFilter meshFilter = transform.gameObject.GetComponent<MeshFilter>();
        MeshRenderer meshRenderer = transform.gameObject.GetComponent<MeshRenderer>();

        if (meshFilter && meshRenderer)
        {
            FindMeshByName(strFilteredObjectName);
            for (int m = 0; m < meshRenderer.materials.Length; m++)
            {
                string strFilteredMaterialName = GetFilteredName(meshRenderer.materials[m].name);
                FindMaterialByName(strFilteredMaterialName);
            }
        }

        for (int k = 0; k < transform.childCount; k++)
        {
            CollectMeshesAndMaterials(binaryWriter, transform.GetChild(k));
        }
    }

    int GetAllTexturesCount()
    {
        int nTextures = 0;
        Scene scene = transform.gameObject.scene;
        GameObject[] rootGameObjects = scene.GetRootGameObjects();
        foreach (GameObject gameObject in rootGameObjects) nTextures += GetTexturesCount(gameObject.transform);
        return (nTextures);
    }

    void Start()
    {
        BinaryWriter binaryWriter = new BinaryWriter(File.Open("Scene.bin", FileMode.Create));

        Scene scene = transform.gameObject.scene;
        GameObject[] rootGameObjects = scene.GetRootGameObjects();

        foreach (GameObject gameObject in rootGameObjects) CollectMeshesAndMaterials(binaryWriter, gameObject.transform);

        binaryWriter.Write("<Meshes>:");
        binaryWriter.Write(m_strMeshNames.Count);
        foreach (string name in m_strMeshNames) binaryWriter.Write(name);
        foreach (string name in m_strMeshNames) print("Mesh: = " + name);
        m_strMeshNames.Clear();

        binaryWriter.Write("<Materials>:");
        binaryWriter.Write(m_strMaterialNames.Count);
        foreach (string name in m_strMaterialNames) binaryWriter.Write(name);
        foreach (string name in m_strMaterialNames) print("Material: = " + name);
        m_strMaterialNames.Clear();

        int nGameObjects = GetAllGameObjectsCount();
        binaryWriter.Write("<GameObjects>:");
        binaryWriter.Write(nGameObjects);

#if (_WITH_TEXTURE)
        int nTextures = GetAllTexturesCount();
        binaryWriter.Write("<Textures>:");
        binaryWriter.Write(nTextures);
#endif
        foreach (GameObject gameObject in rootGameObjects) WriteFrameHierarchy(binaryWriter, gameObject.transform);

        binaryWriter.Write("</GameObjects>");

        binaryWriter.Flush();
        binaryWriter.Close();

        print("Scene Write Completed");
    }
}
