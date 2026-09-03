/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:
Version:    1.0
Date:		2020-7-15
Description:委外加工费发送
**************************************************/
//框架用头文件
#include "stdafx.h"
//程序用头文件
#include "epex.h"



//外部函数声明
CString f_mm0099_sap_seq(CDbConnection *conn);//生产SAP抛帐序列号
int f_cm_mesac2_snd(EIClass * bcls_rec, EIClass * bcls_ret, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义

	/* 程序内部变量 */
	int	doFlag = 0;

	/* 业务变量 */
	CString	datetime("");
	CString	datenow("");
	/*赋电文号*/
	CString	tc_no = "MESAC2";
	CString	retpo = " ";
	
	int ret = 0;
	//EIClass bcls_temp;

	/* 实体类定义 */
	CModel tcaais7("TCAAIS7");

	CString		sqlstr("");              // 数据库SQL操作字符串
	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);

	try
	{
		// 生成电文发送对象
		EPEX epex;

		/*初始化*/
		if (epex.Initialize(tc_no) < 0)
		{
			strcpy(s.msg, _RES("PS00S0000686")/*电文初始化失败。*/);
			sprintf(s.sysmsg, "电文初始化出错！");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		datetime = CDateTime::Now().ToString("yyyyMMddHH24missff6"); 
		datenow = CDateTime::Now().ToString("yyyyMMdd");

		for (int i = 0; i < bcls_rec->Tables[0].Rows.get_Count(); i++)
		{
			tcaais7.Reset();
			tcaais7.MergeFrom(bcls_rec->Tables[0].Rows[i]);
			retpo = " ";
			if (bcls_rec->Tables[0].Columns.Contains("RETPO"))
			{
				retpo = bcls_rec->Tables[0].Rows[i]["RETPO"].ToString();
			}			
			epex.SetValue("HEADDATA", "MSG_SEQ_NO", 0, f_mm0099_sap_seq(conn));//作业类型
			epex.SetValue("HEADDATA", "ZJOB_TYPE", 0, "1");//作业类型
			epex.SetValue("HEADDATA", "ZEXTS_TYPE", 0, "MES");//外围系统类型			
			epex.SetValue("HEADDATA", "ZEXTS_DOC", 0, datetime);//支撑系统单号
			epex.SetValue("HEADDATA", "BLDAT", 0, tcaais7["PROD_DATE"].ToString());//凭证中的凭证日期
			epex.SetValue("HEADDATA", "BUDAT", 0, datenow);//凭证中的过账日期
			epex.SetValue("HEADDATA", "UNSEZ", 0, " ");//我们的参考（合同号）
			epex.SetValue("HEADDATA", "BSART", 0, " ");//采购凭证类型
			epex.SetValue("HEADDATA", "LIFNR", 0, tcaais7["OUT_UNIT_CODE"].ToString());//供应商
			epex.SetValue("HEADDATA", "EKORG", 0, "6661");//采购组织
			epex.SetValue("HEADDATA", "EKGRP", 0, tcaais7["RECV_DEPT"].ToString());//采购组 三家马特
			epex.SetValue("HEADDATA", "BUKRS", 0, "6667");//公司代码
			epex.SetValue("HEADDATA", "XBLNR", 0," ");//外部交货单
			epex.SetValue("HEADDATA", "USNAM", 0, s.userid);//用户名
			epex.SetValue("HEADDATA", "BKTXT", 0, " ");//凭证抬头文本
			epex.SetValue("ITEM", "ZEXTS_DOC_ITEM", 0, "0001");//支撑系统单行项目号
			epex.SetValue("ITEM", "KNTTP", 0, "F");//科目分配类别
			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default:
				sqlstr = " SELECT CODE_DESC_4_CONTENT FROM TEP0002 "
					" where 1=1 AND CODE=@code  AND code_class='CAW2'"
					;
				break;
			}
			cmd_inq.SetCommandText(sqlstr);
			cmd_inq.Parameters.Set("code", tcaais7["PROC_MODE"].ToString());
			cmd_inq.ExecuteReader();
			while (cmd_inq.Read())
			{
				epex.SetValue("ITEM", "MATNR", 0, cmd_inq.GetString(1));//物料编码
			}
			epex.SetValue("ITEM", "TXZ01", 0, " ");//短文本
			epex.SetValue("ITEM", "MATKL", 0, " ");//物料组
			epex.SetValue("ITEM", "WERKS", 0, "6767");//工厂
			epex.SetValue("ITEM", "LGORT", 0, " ");//库存地点
			epex.SetValue("ITEM", "MEINS", 0, " ");//单位
			epex.SetValue("ITEM", "EINDT", 0, tcaais7["PROD_DATE"].ToString());//交货日期
			epex.SetValue("ITEM", "RETPO", 0, retpo);//退货项目
			epex.SetValue("ITEM", "UMSON", 0, " ");//免费项目
			epex.SetValue("ITEM", "MWSKZ", 0, tcaais7["TAX_RATE_NAME"].ToString()); //税码
			epex.SetValue("ITEM", "MENGE", 0, tcaais7["WT"].ToDecimal().Round(3));//数量(小数点3位）
			if (tcaais7["WT"].ToDecimal().Round(3) < 0)
			{
				epex.SetValue("ITEM", "MENGE", 0, 0-tcaais7["WT"].ToDecimal().Round(3));//数量(小数点3位）
				if (retpo == "X")
				{
					epex.SetValue("ITEM", "RETPO", 0, " ");//退货项目
				}
				else
				{
					epex.SetValue("ITEM", "RETPO", 0, "X");//退货项目
				}
			}
			
			epex.SetValue("ITEM", "NETPR", 0, tcaais7["PRICE_UNIT"].ToDecimal().Round(2));//净价(小数点2位）
			epex.SetValue("ITEM", "CURRENCY", 0, "CNY"); //货币
			epex.SetValue("ITEM", "AUFNR", 0, tcaais7["PLAN_NO"].ToString());//内部订单号
			epex.SetValue("ITEM", "KOSTL", 0, " ");//成本中心
			epex.SetValue("ITEM", "CHARG", 0, " ");//批次
			epex.SetValue("ITEM", "SGTXT", 0, " ");//项目文本

			epex.SendTele();
		}
		// 释放
		epex.Uninitialize();
	}

	catch (CDbException& ex)  //捕获数据库操作异常
	{
		CFormattable arguments[] = { ex.GetCode() };
		CMessageFormat::Format(s.msg, _RES("GCRSS0000006")/*数据库处理出错，sqlcode=[{0}]。请联系系统维护人员。*/, arguments, 1);

		CString str = sqlstr + "\r\n" + ex.GetMsg();
		//EDLog(1,1, "error=[%s]", (const char*)str );

		strncpy(s.sysmsg, (const char*)str, sizeof(s.msg) - 1);  //返回前台，与EI.EIInfo对象的sys_info.sysmsg参数对应
		s.flag = -1;
		doFlag = -1;                 //数据库异常时返回-1，事务将被回滚
	}
	catch (CApplicationException& ex)  //捕获应用错误
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}
	catch (CException& ex)
	{
		strncpy(s.msg, (const char*)ex.GetMsg(), sizeof(s.msg) - 1);
		s.flag = ex.GetCode();
		doFlag = -1;
	}

	return doFlag;

}