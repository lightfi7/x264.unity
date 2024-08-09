using System.Collections;
using System.Collections.Generic;
using System.Net.Sockets;
using System.Net;
using UnityEngine;
using UnityEngine.UI;
using System;
using System.Reflection;
using System.Runtime.InteropServices;



public class CameraCapture : MonoBehaviour
{

    [DllImport("x264.unity.dll", CallingConvention = CallingConvention.Cdecl)]
    private static extern int InitializeEncoder(int w, int h);

    [DllImport("x264.unity.dll", CallingConvention = CallingConvention.Cdecl)]
    private static extern int EncodeFrame(int w, int h, byte[] in_buffer, out IntPtr out_buffer);
    
    [DllImport("x264.unity.dll", CallingConvention = CallingConvention.Cdecl)]
    private static extern void DisposeEncoder();


    public Camera sceneCamera; // Reference to the scene camera
    public int textureWidth = 1280; // Width of the RenderTexture
    public int textureHeight = 720; // Height of the RenderTexture
    public string remoteIPAddress = "127.0.0.1"; // IP address of the remote machine
    public int remotePort = 5000; // Port number of the remote machine
    public int frequency = 10;

    private float lastTime = 0f;
    private RenderTexture renderTexture;
    private Texture2D texture2D;
    private UdpClient udpClient;



    void Start()
    {

        lastTime = Time.time;

        // Create a new RenderTexture
        renderTexture = new RenderTexture(textureWidth, textureHeight, 24);

        // Assign the RenderTexture to the scene camera
        sceneCamera.targetTexture = renderTexture;

        // Create a new Texture2D to read the RenderTexture
        texture2D = new Texture2D(textureWidth, textureHeight, TextureFormat.RGB24, false);

        // Initialize the UdpClient
        udpClient = new UdpClient();

        InitializeEncoder(textureWidth, textureHeight);
    }

    void Update()
    {
        // Capture the image from the scene camera
        if (Time.time - lastTime > 1f / frequency)
        {
            lastTime = Time.time;
            CaptureAndSendImage();
        }
    }

    void CaptureAndSendImage()
    {
        // Set the active RenderTexture
        RenderTexture.active = renderTexture;

        // Read the RenderTexture into the Texture2D
        texture2D.ReadPixels(new Rect(0, 0, textureWidth, textureHeight), 0, 0);
        texture2D.Apply();

        // Reset the active RenderTexture
        RenderTexture.active = null;

        // Convert the Texture2D to a byte array
        byte[] imageBytes = texture2D.GetRawTextureData();

        Debug.Log(imageBytes.Length);

        // Call the native method
        IntPtr outBuffer;
        int frameSize = EncodeFrame(textureWidth, textureHeight, imageBytes, out outBuffer);

        Debug.Log(frameSize);
        if (frameSize > 0)
        {
            byte[] encodedFrame = new byte[frameSize];
            Marshal.Copy(outBuffer, encodedFrame, 0, frameSize);

            // Use the encoded frame (e.g., send it over the network)
            SendUdpMessage(encodedFrame);

            // Free the memory allocated in the DLL
            Marshal.FreeHGlobal(outBuffer);
        }

    }

    void SendUdpMessage(byte[] message)
    {
        // Create an endpoint for the remote machine
        IPEndPoint remoteEndPoint = new IPEndPoint(IPAddress.Parse(remoteIPAddress), remotePort);

        // Send the message asynchronously
        udpClient.SendAsync(message, message.Length, remoteEndPoint);
    }

    void OnDestroy()
    {
        // Clean up the RenderTexture and UdpClient when the script is destroyed
        if (renderTexture != null)
        {
            renderTexture.Release();
            renderTexture = null;
        }

        if (udpClient != null)
        {
            udpClient.Close();
            udpClient = null;
        }

        DisposeEncoder();
    }
}
