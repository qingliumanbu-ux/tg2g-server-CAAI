/*************************************************
Copyright:	Baosight Software LTD.co Copyright (c) 2012
Author:
Version:    1.0
Date:		2020-8-17
Description:能源消耗发送
**************************************************/
//框架用头文件
#include "stdafx.h"
//程序用头文件
#include "epex.h"



//外部函数声明
CString f_mm0099_sap_seq(CDbConnection *conn);//生产SAP抛帐序列号
int f_cm_mesac3_snd(CString account_period, CString dept_code, CDbConnection * conn)
{
	CTracer log(__FUNCTION__);  // 系统日志，必须在代码段开始处定义

	/* 程序内部变量 */
	int	doFlag = 0;


	/* 业务变量 */
	CString	datetime("");
	CString	plan_no("");
	CString	equ_no("");
	CString datenow = CDateTime::Now().ToString("yyyyMMddHHmmss");

	/*赋电文号*/
	CString	tc_no = "MESAC3";
	int ret = 0;
	//EIClass bcls_temp;

	/* 实体类定义 */
	CModel tcaai04("TCAAI04");

	CString		sqlstr("");              // 数据库SQL操作字符串
	/* 数据库操作类定义 */
	CDbCommand cmd_inq(conn);
	CDbCommand cmd_inq_1(conn);

	try
	{
		// 生成电文发送对象
		EPEX epex(&s, conn);
		EDLog(1, 1, "电文开始");
		
		datetime = CDateTime::Now().ToString("yyyyMMdd");

		/*初始化*/
		if (epex.Initialize(tc_no) < 0)
		{
			strcpy(s.msg, _RES("PS00S0000686")/*电文初始化失败。*/);
			sprintf(s.sysmsg, "电文初始化出错！");
			throw CApplicationException(-1, s.msg, log.Location);
		}

		switch (conn->DatabaseKind)
		{
		case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
		case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
		case DB_KIND_MSSQL:	        // MS SQL Server数据库
		case DB_KIND_ORACLE:	    // Oracle 数据库
		default:
			sqlstr = " SELECT DISTINCT PLAN_NO,EQU_NO,SUB_BACKLOG_CODE "
				" FROM tcaac20 "
				" WHERE 1=1 "
				" and SEND_FLAG!='1'"
				" AND MAT_CODE IN ('D01','D02','D03','D04','E01','C01','C03','C02','E02','F04','F01','F02','F03','F05','E03') "
				" AND account_period=@account_period "
				" AND DEPT_CODE=@dept_code "
				;
			break;
		}
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.Parameters.Set("dept_code", dept_code);
		cmd_inq.ExecuteReader();
		int i = 0;
		while (cmd_inq.Read())
		{
			epex.SetValue("ZPPT_NY", "MSG_SEQ_NO", i, f_mm0099_sap_seq(conn));//序号	

			//炉号、计划号
		
			if (epex.SetValue("ZPPT_NY", "PLAN_NO", i, cmd_inq.GetString(1).Trim()) < 0)
			{
				Log::Trace("", __FUNCTION__, "PLAN_NO = [{0}]", cmd_inq.GetString(1).Trim());
				throw CApplicationException(-1, s.msg, log.Location);
			}
			plan_no = cmd_inq.GetString(1).Trim(); 

			
			//设备号序号
			if (dept_code == "S")
			{
				sqlstr = "SELECT pono FROM TPSSM41"
					" WHERE  HEAT_NO = @heat_no"
					;
				//Log::Trace("", "", "sqlstr = [{0}],heat_no=【{1}】", sqlstr, cmd_inq.GetString(1));
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("heat_no", cmd_inq.GetString(1));
				cmd_inq_1.ExecuteReader();
				while (cmd_inq_1.Read())
				{
					if (epex.SetValue("ZPPT_NY", "PLAN_NO", i, cmd_inq_1.GetString(1)) < 0)
					{
						Log::Trace("", __FUNCTION__, "PLAN_NO = [{0}]", cmd_inq.GetString(1).Trim());
						throw CApplicationException(-1, s.msg, log.Location);
					}
					
				}
				cmd_inq_1.Close();

				//Log::Trace("", "", "1111111111111111");

				sqlstr = "SELECT charge_no,dev_code FROM TPSSM42"
					" WHERE dev_code = @equ_no"
					" AND HEAT_NO = @heat_no"
					;
				Log::Trace("", "", "sqlstr = [{0}],heat_no=【{1}】,dev_code = [{2}]", sqlstr, cmd_inq.GetString(1), cmd_inq.GetString(2));
				cmd_inq_1.SetCommandText(sqlstr);
				cmd_inq_1.Parameters.Set("equ_no", cmd_inq.GetString(2));
				cmd_inq_1.Parameters.Set("heat_no", cmd_inq.GetString(1));
				cmd_inq_1.ExecuteReader();
				while (cmd_inq_1.Read())
				{
					//Log::Trace("", "", "33333333333333");
					if (epex.SetValue("ZPPT_NY", "EQUXUHAO", i, cmd_inq_1.GetDecimal(1)) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				//Log::Trace("", "", "22222222222222");
				cmd_inq_1.Close();
			}
			else
			{
				if (epex.SetValue("ZPPT_NY", "EQUXUHAO", i,"1") < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}			

			Log::Trace("", "", "3333333333333");
			//设备号
			if (epex.SetValue("ZPPT_NY", "EQU_NO", i, cmd_inq.GetString(2).Trim()) < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
			equ_no = cmd_inq.GetString(2).Trim(); 

			//if (dept_code == "R")
			//{
			//	sqlstr = "SELECT equ_no FROM HPSBWB1 "
			//		" WHERE PLAN_NO = @plan_no"
			//		;
			//	cmd_inq_1.SetCommandText(sqlstr);
			//	cmd_inq_1.Parameters.Set("plan_no", cmd_inq.GetString(1));
			//	cmd_inq_1.ExecuteReader();
			//	while (cmd_inq_1.Read())
			//	{
			//		if (epex.SetValue("ZPPT_NY", "EQU_NO", i, cmd_inq_1.GetString(1)) < 0)
			//		{
			//			throw CApplicationException(-1, s.msg, log.Location);
			//		}
			//	/*	equ_no = cmd_inq_1.GetString(1);*/
			//	}
			//	cmd_inq_1.Close();
			//}

			//if (dept_code == "X")
			//{
			//	sqlstr = "SELECT equ_no FROM HPSBWA1 "
			//		" WHERE ROLL_PLAN_NO = @plan_no"
			//		;
			//	cmd_inq_1.SetCommandText(sqlstr);
			//	cmd_inq_1.Parameters.Set("plan_no", cmd_inq.GetString(1));
			//	cmd_inq_1.ExecuteReader();
			//	while (cmd_inq_1.Read())
			//	{
			//		if (epex.SetValue("ZPPT_NY", "EQU_NO", i, cmd_inq_1.GetString(1)) < 0)
			//		{
			//			throw CApplicationException(-1, s.msg, log.Location);
			//		}
			//		//equ_no = cmd_inq_1.GetString(1);
			//	}
			//	cmd_inq_1.Close();
			//}
			//凭证日期
			if (epex.SetValue("ZPPT_NY", "BLDAT", i,datetime) < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}


			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
				sqlstr = " selecT E_DATETIME FROM  tcaac15 where account_period=@account_period ";
				break;
			}
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("account_period", account_period);
			cmd_inq_1.ExecuteReader();
			while (cmd_inq_1.Read())
			{
				//过账日期
				if (epex.SetValue("ZPPT_NY", "BUDAT", i, cmd_inq_1.GetString(1)) < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}
			}
			cmd_inq_1.Close();
			
			//工厂
			if (epex.SetValue("ZPPT_NY", "FACTORY_DIV", i, "6767") < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}
			//操作类型
			if (epex.SetValue("ZPPT_NY", "MSG_TYPE", i, "I") < 0)
			{
				throw CApplicationException(-1, s.msg, log.Location);
			}


			switch (conn->DatabaseKind)
			{
			case DB_KIND_DB2:	        // DB2 数据库（未开Oracle兼容）
			case DB_KIND_DB2_ORACLE:	// DB2 数据库（开Oracle兼容）
			case DB_KIND_MSSQL:	        // MS SQL Server数据库
			case DB_KIND_ORACLE:	    // Oracle 数据库
			default:
				sqlstr = " SELECT PLAN_NO,EQU_NO,DEPT_CODE,MAT_CODE,SUM(WT) "
					" FROM tcaac20 "
					" WHERE 1=1 "
					" AND MAT_CODE IN ('D01','D02','D03','D04','E01','C01','C03','C02','E02','F04','F01','F02','F03','F05','E03') "
					" AND PLAN_NO=@plan_no "
					//" and SEND_FLAG!='1'"
					" AND account_period=@account_period "
					" AND DEPT_CODE=@dept_code "
					" AND EQU_NO=@equ_no"
					" GROUP BY PLAN_NO,EQU_NO,DEPT_CODE,MAT_CODE "
					;
				break;
			}
			cmd_inq_1.SetCommandText(sqlstr);
			cmd_inq_1.Parameters.Set("account_period", account_period);
			cmd_inq_1.Parameters.Set("dept_code", dept_code);
			cmd_inq_1.Parameters.Set("plan_no", cmd_inq.GetString(1));
			cmd_inq_1.Parameters.Set("equ_no", equ_no);
			cmd_inq_1.ExecuteReader();
			while (cmd_inq_1.Read())
			{

				if (cmd_inq_1.GetString(4).Trim() == "C01")
				{
					//动力-氧气O2
					if (epex.SetValue("ZPPT_NY", "LSTAR1", i, "C01") < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-氧气O2用量
					if (epex.SetValue("ZPPT_NY", "MBGBTR1", i, cmd_inq_1.GetDecimal(5)) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-氧气O2单位
					if (epex.SetValue("ZPPT_NY", "MEINB1", i, "M3") < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}

				if (cmd_inq_1.GetString(4).Trim() == "C02")
				{
					//动力-氮气N2
					if (epex.SetValue("ZPPT_NY", "LSTAR2", i, "C02") < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-氮气N2用量
					if (epex.SetValue("ZPPT_NY", "MBGBTR2", i, cmd_inq_1.GetDecimal(5)) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-氮气N2单位
					if (epex.SetValue("ZPPT_NY", "MEINB2", i, "M3") < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				if (cmd_inq_1.GetString(4).Trim() == "C03")
				{
					//动力-氩气AR
					if (epex.SetValue("ZPPT_NY", "LSTAR3", i, "C03") < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-氩气AR用量
					if (epex.SetValue("ZPPT_NY", "MBGBTR3", i, cmd_inq_1.GetDecimal(5)) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-氩气AR单位
					if (epex.SetValue("ZPPT_NY", "MEINB3", i, "M3") < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				if (cmd_inq_1.GetString(4).Trim() == "D01")
				{
					//动力-尖电
					if (epex.SetValue("ZPPT_NY", "LSTAR4", i, cmd_inq_1.GetString(4).Trim()) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-尖电用量
					if (epex.SetValue("ZPPT_NY", "MBGBTR4", i, cmd_inq_1.GetDecimal(5)) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-尖电单位
					if (epex.SetValue("ZPPT_NY", "MEINB4", i, "KWH") < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				if (cmd_inq_1.GetString(4).Trim() == "D02")
				{
					//动力-峰电
					if (epex.SetValue("ZPPT_NY", "LSTAR5", i, cmd_inq_1.GetString(4).Trim()) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-峰电用量
					if (epex.SetValue("ZPPT_NY", "MBGBTR5", i, cmd_inq_1.GetDecimal(5)) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-峰电单位
					if (epex.SetValue("ZPPT_NY", "MEINB5", i, "KWH") < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				if (cmd_inq_1.GetString(4).Trim() == "D03")
				{
					//动力-平电
					if (epex.SetValue("ZPPT_NY", "LSTAR6", i, cmd_inq_1.GetString(4).Trim()) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-平电用量
					if (epex.SetValue("ZPPT_NY", "MBGBTR6", i, cmd_inq_1.GetDecimal(5)) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-平电单位
					if (epex.SetValue("ZPPT_NY", "MEINB6", i, "KWH") < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				if (cmd_inq_1.GetString(4).Trim() == "D04")
				{
					//动力-谷电
					if (epex.SetValue("ZPPT_NY", "LSTAR7", i, cmd_inq_1.GetString(4).Trim()) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-谷电用量
					if (epex.SetValue("ZPPT_NY", "MBGBTR7", i, cmd_inq_1.GetDecimal(5)) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-谷电单位
					if (epex.SetValue("ZPPT_NY", "MEINB7", i, "KWH") < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				if (cmd_inq_1.GetString(4).Trim() == "E01")
				{
					//动力-天然气
					if (epex.SetValue("ZPPT_NY", "LSTAR8", i, cmd_inq_1.GetString(4).Trim()) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-天然气用量
					if (epex.SetValue("ZPPT_NY", "MBGBTR8", i, cmd_inq_1.GetDecimal(5)) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-天然气单位
					if (epex.SetValue("ZPPT_NY", "MEINB8", i, "M3") < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				if (cmd_inq_1.GetString(4).Trim() == "E02")
				{
					//动力-压缩空气
					if (epex.SetValue("ZPPT_NY", "LSTAR9", i, cmd_inq_1.GetString(4).Trim()) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-压缩空气用量
					if (epex.SetValue("ZPPT_NY", "MBGBTR9", i, cmd_inq_1.GetDecimal(5)) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-压缩空气单位
					if (epex.SetValue("ZPPT_NY", "MEINB9", i, "M3") < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				if (cmd_inq_1.GetString(4).Trim() == "E03")
				{
					//动力-蒸汽
					if (epex.SetValue("ZPPT_NY", "LSTAR10", i, cmd_inq_1.GetString(4).Trim()) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-蒸汽用量
					if (epex.SetValue("ZPPT_NY", "MBGBTR10", i, cmd_inq_1.GetDecimal(5)) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-蒸汽单位
					if (epex.SetValue("ZPPT_NY", "MEINB10", i, "M3") < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				if (cmd_inq_1.GetString(4).Trim() == "F01")
				{
					//动力-新水
					if (epex.SetValue("ZPPT_NY", "LSTAR11", i, cmd_inq_1.GetString(4).Trim()) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-新水用量
					if (epex.SetValue("ZPPT_NY", "MBGBTR11", i, cmd_inq_1.GetDecimal(5)) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-新水单位
					if (epex.SetValue("ZPPT_NY", "MEINB11", i, "TON") < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				if (cmd_inq_1.GetString(4).Trim() == "F02")
				{
					//动力-一级除盐水
					if (epex.SetValue("ZPPT_NY", "LSTAR12", i, cmd_inq_1.GetString(4).Trim()) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-一级除盐水用量
					if (epex.SetValue("ZPPT_NY", "MBGBTR12", i, cmd_inq_1.GetDecimal(5)) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-一级除盐水单位
					if (epex.SetValue("ZPPT_NY", "MEINB12", i, "TON") < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				if (cmd_inq_1.GetString(4).Trim() == "F03")
				{
					//动力-二级除盐水
					if (epex.SetValue("ZPPT_NY", "LSTAR13", i, cmd_inq_1.GetString(4).Trim()) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-二级除盐水用量
					if (epex.SetValue("ZPPT_NY", "MBGBTR13", i, cmd_inq_1.GetDecimal(5)) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-二级除盐水单位
					if (epex.SetValue("ZPPT_NY", "MEINB13", i, "TON") < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				if (cmd_inq_1.GetString(4).Trim() == "F04")
				{
					//动力-市政自来水
					if (epex.SetValue("ZPPT_NY", "LSTAR14", i, cmd_inq_1.GetString(4).Trim()) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-市政自来水用量
					if (epex.SetValue("ZPPT_NY", "MBGBTR14", i, cmd_inq_1.GetDecimal(5)) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-市政自来水单位
					if (epex.SetValue("ZPPT_NY", "MEINB14", i, "TON") < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
				}
				if (cmd_inq_1.GetString(4).Trim() == "F05")
				{
					//动力-循环水
					if (epex.SetValue("ZPPT_NY", "LSTAR15", i, cmd_inq_1.GetString(4).Trim()) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-循环水用量
					if (epex.SetValue("ZPPT_NY", "MBGBTR15", i, cmd_inq_1.GetDecimal(5)) < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}
					//动力-循环水单位
					if (epex.SetValue("ZPPT_NY", "MEINB15", i, "TON") < 0)
					{
						throw CApplicationException(-1, s.msg, log.Location);
					}

				}
				
			}
			cmd_inq_1.Close();

			//by 20201029 不合并，逐条发
				if (epex.SendTele() < 0)
				{
					throw CApplicationException(-1, s.msg, log.Location);
				}

				// 释放
				epex.Uninitialize();
				/*初始化*/
				if (epex.Initialize(tc_no) < 0)
				{
					strcpy(s.msg, _RES("PS00S0000686")/*电文初始化失败。*/);
					sprintf(s.sysmsg, "电文初始化出错！");
					throw CApplicationException(-1, s.msg, log.Location);
				}


			//i++;

			//if (i % 8 == 0)  //每20条记录进行发送，分批
			//{
			//	i = 0;
			//	if (epex.SendTele() < 0)
			//	{
			//		throw CApplicationException(-1, s.msg, log.Location);
			//	}

			//	// 释放
			//	epex.Uninitialize();
			//	/*初始化*/
			//	if (epex.Initialize(tc_no) < 0)
			//	{
			//		strcpy(s.msg, _RES("PS00S0000686")/*电文初始化失败。*/);
			//		sprintf(s.sysmsg, "电文初始化出错！");
			//		throw CApplicationException(-1, s.msg, log.Location);
			//	}
			//}

			
		}
		cmd_inq.Close();

		////发送电文
		//if (i >= 1)
		//{

		//	if (epex.SendTele() < 0)
		//	{
		//		throw CApplicationException(-1, s.msg, log.Location);
		//	}
		//}
		//// 释放
		//epex.Uninitialize();


		sqlstr = "update tcaac20 set SEND_FLAG = '1',SEND_TIME = @datenow,TC_NO='MESAC3'"
			" WHERE 1=1 "
			" and SEND_FLAG!='1'"
			" AND MAT_CODE IN ('D01','D02','D03','D04','E01','C01','C03','C02','E02','F04','F01','F02','F03','F05','E03') "
			" AND account_period=@account_period "
			" AND DEPT_CODE=@dept_code "
			;
		cmd_inq.SetCommandText(sqlstr);
		cmd_inq.Parameters.Set("account_period", account_period);
		cmd_inq.Parameters.Set("dept_code", dept_code); 
		cmd_inq.Parameters.Set("datenow", datenow);
		cmd_inq.ExecuteNonQuery();
		cmd_inq.Close();

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