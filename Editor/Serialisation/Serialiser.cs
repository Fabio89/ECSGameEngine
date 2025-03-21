using System.Diagnostics;
using System.IO;
using System.Runtime.Serialization;

namespace Editor.Serialisation;

public static class Serialiser
{
    public static void WriteToFile<T>(T obj, string path)
    {
        try
        {
            using var fs = new FileStream(path, FileMode.Create);
            var serialiser = new DataContractSerializer(typeof(T));
            serialiser.WriteObject(fs, obj);
        }
        catch (Exception e)
        {
            Debug.WriteLine(e);
            throw;
        }
    }
    
    public static T? ReadFromFile<T>(string path) where T : class
    {
        try
        {
            using var fs = new FileStream(path, FileMode.Create);
            var serialiser = new DataContractSerializer(typeof(T));
            return serialiser.ReadObject(fs) as T;
        }
        catch (Exception e)
        {
            Debug.WriteLine(e);
            return null;
        }
    }
}