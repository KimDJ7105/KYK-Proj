//#define _WITH_TEXTURE

using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

using System.IO;
using UnityEditor;
using System.Text;

public class BinaryObjectWriter : MonoBehaviour
{
    private List<string> m_strGameObjectNames = new List<string>();
#if (_WITH_TEXTURE)
    private List<string> m_strTextureNames = new List<string>();
#endif

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

    void WriteColor(BinaryWriter binaryWriter, Color color)
    {
        binaryWriter.Write(color.r);
        binaryWriter.Write(color.g);
        binaryWriter.Write(color.b);
        binaryWriter.Write(color.a);
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

    void WriteFloat(BinaryWriter binaryWriter, string strHeader, float f)
    {
        binaryWriter.Write(strHeader);
        binaryWriter.Write(f);
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

    void WriteMaterials(BinaryWriter binaryWriter, Material[] materials)
    {
        binaryWriter.Write("<Materials>:");
        binaryWriter.Write(materials.Length);

        for (int i = 0; i < materials.Length; i++)
        {
            WriteObjectName(binaryWriter, "<Material>:", materials[i].name);

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
            binaryWriter.Write("</Material>");
        }

        binaryWriter.Write("</Materials>");
    }

    void WriteMesh(BinaryWriter binaryWriter, Mesh mesh)
    {
        WriteObjectName(binaryWriter, "<Mesh>:", mesh.name);

        BinaryWriter binaryMeshWriter = new BinaryWriter(File.Open(mesh.name + ".bin", FileMode.Create));

        WriteBoundingBox(binaryMeshWriter, "<BoundingBox>:", mesh.bounds); //AABB

        Write3DVectors(binaryMeshWriter, "<Positions>:", mesh.vertices);
        Write3DVectors(binaryMeshWriter, "<Normals>:", mesh.normals);
#if (_WITH_TEXTURE)
        Write2DVectors(binaryMeshWriter, "<TextureCoords>:", mesh.uv);
#endif
        WriteIntegers(binaryMeshWriter, "<Indices>:", mesh.triangles);

        WriteSubMeshes(binaryMeshWriter, "<SubMeshes>:", mesh);

        binaryMeshWriter.Write("</Mesh>");

        binaryMeshWriter.Flush();
        binaryMeshWriter.Close();
    }

#if (_WITH_TEXTURE)
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
#endif

    void Start()
    {
        BinaryWriter objectBinaryWriter = new BinaryWriter(File.Open(string.Copy(transform.name).Replace(" ", "_") + "Object.bin", FileMode.Create));

        WriteObjectName(objectBinaryWriter, "<GameObject>:", transform.name);
        WriteWorldMatrix(objectBinaryWriter, transform);

        MeshFilter meshFilter = transform.gameObject.GetComponent<MeshFilter>();
        MeshRenderer meshRenderer = transform.gameObject.GetComponent<MeshRenderer>();
        if (meshFilter && meshRenderer)
        {
            WriteMaterials(objectBinaryWriter, meshRenderer.materials);
            WriteMesh(objectBinaryWriter, meshFilter.sharedMesh);
        }

        objectBinaryWriter.Flush();
        objectBinaryWriter.Close();

        print("Mesh Write Completed");
    }
}
