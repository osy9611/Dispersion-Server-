using System.Collections;
using System.Collections.Generic;
using UnityEngine.SceneManagement;
using UnityEngine;

public class SceneChange : MonoBehaviour
{
    public TransportTCP transportTCP;
    // Start is called before the first frame update
    void Start()
    {
        transportTCP = FindObjectOfType<TransportTCP>();
    }

    // Update is called once per frame
    void Update()
    {
        if(transportTCP.LogIn)
        {
            SceneManager.LoadScene("Robby");
        }
    }
}
