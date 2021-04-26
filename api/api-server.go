package main

import (
	"fmt"
	"log"
	"net/http"
	"os"
	"strconv"
	"time"

	"github.com/gin-gonic/gin"
)

type HomInfo struct {
	OutsideTemp   float64 `json:"outsidetemp"`
	OutsideHum    uint    `json:"outsidehum" `
	InsideTemp    float64 `json:"insidetemp" `
	InsideHum     uint    `json:"insidehum" `
	Latency       float64 `json:"latency"`
	LastHeartbeat string  `json:"unixtime"`
}
type UpdateRequest struct {
	Data HomInfo `json:"data" binding:"required"`
	User string  `json:"user" binding:"required"`
	Key  string  `json:"key" binding:"required" `
}

var router *gin.Engine
var HomeSenor HomInfo

func init() {
	gin.SetMode(gin.ReleaseMode)
	router = gin.New()
	read_only_group := router.Group("/api/get")
	{
		read_only_group.Use(CORSMiddleware())
		read_only_group.GET("/homeinfo", func(c *gin.Context) {
			c.JSON(200, HomeSenor)
		})
	}
	post_data_group := router.Group("/api/post")
	{
		post_data_group.Use(CORSMiddleware())
		post_data_group.POST("/updatehomeinfo", UpdateHome)
	}

}
func UpdateHome(c *gin.Context) {
	var req UpdateRequest
	if err := c.ShouldBindJSON(&req); err != nil {
		c.JSON(http.StatusBadRequest, gin.H{"error": err.Error()})
		return

	} else if (req.User != "apiser") || (req.Key != "a-api-key") { // basic auth :D
		c.JSON(http.StatusBadRequest, gin.H{"error": "Who Are You! please leave me alone"})
		return
	}
	req.Data.LastHeartbeat = time.Now().Format(time.RFC3339)
	req_data := UpdateRequest{Data: req.Data, User: req.User, Key: req.Key}
	HomeSenor = req_data.Data
	c.JSON(http.StatusOK, gin.H{"data": HomeSenor})
}

func CORSMiddleware() gin.HandlerFunc {
	return func(c *gin.Context) {

		c.Header("Access-Control-Allow-Origin", "*")
		c.Header("Access-Control-Allow-Credentials", "true")
		c.Header("Access-Control-Allow-Headers", "Content-Type, Content-Length, Accept-Encoding, X-CSRF-Token, Authorization, accept, origin, Cache-Control, X-Requested-With")
		c.Header("Access-Control-Allow-Methods", "POST,HEAD,GET")

		if c.Request.Method == "OPTIONS" {
			c.AbortWithStatus(204)
			return
		}

		c.Next()
	}
}

func main() {
	if len(os.Args) <= 1 {
		fmt.Println("server runs default port :8080")
		router.Run(":8080")
	}
	if len(os.Args) == 2 {
		i, err := strconv.Atoi(os.Args[1])
		if err != nil {
			log.Fatalln("Wrong port type ")
		}
		if i > 79 || i < 65500 {
			fmt.Println("Router Workson :", os.Args[1])
			router.Run(":" + os.Args[1])

		} else {
			log.Fatalln("Wrong Port Range")
		}
	} else {
		log.Fatalln("Wron type argument is entered \n example use './p_name 8080'\n ")
	}
}
